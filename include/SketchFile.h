#pragma once

#include "pch.h"
#include "config.h"
#include "FileContent.h"
#include "Layer.h"

/// <summary>
/// This class guarantees, that always at least 1 layer exists
/// </summary>
class SketchFile {

	FileContent content;

	bool fileChanged = false;
	std::string filename = DEFAULT_FILENAME;	// Filename contains extension
	std::string fileLocation = "";

public:
	SketchFile() {

	}

	void PushLayer() {
		content.PushLayer();
		fileChanged = true;
	}

	void PushLayer(const std::string& name) {
		content.PushLayer(name);
		fileChanged = true;
	}

	void PushLayer(Layer&& layer) {
		content.PushLayer(std::move(layer));
		fileChanged = true;
	}

	void GeneratePreviews() {
		content.GeneratePreviews();
	}

	const std::vector<Layer>& GetLayers() {
		return content.GetLayers();
	}

	const Layer& GetActiveLayer() {
		return content.GetActiveLayer();
	}

	Layer DuplicateActiveLayer() {
		return content.GetActiveLayer().Duplicate();
	}

	bool SetLayerName(LayerID id, const char* name) {
		auto layer = content.FindLayer(id);

		if (layer.has_value()) {	// Layer was found
			layer->get().name = name;
			fileChanged = true;
			return true;
		}

		return false;
	}

	void UndoAction() {
		content.GetActiveLayer().UndoAction();
	}

	bool MoveLayerFront(LayerID id) {
		if (!content.MoveLayerFront(id)) {
			return false;
		}
		fileChanged = true;
		return true;
	}

	bool MoveLayerBack(LayerID id) {
		if (!content.MoveLayerBack(id)) {
			return false;
		}
		fileChanged = true;
		return true;
	}

	bool RemoveLayer(LayerID id) {
		if (!content.RemoveLayer(id)) {
			return false;
		}
		fileChanged = true;
		return true;
	}

	bool ActivateLayer(LayerID id) {
		return content.ActivateLayer(id);
	}

	void AddShape(enum class ShapeType type, glm::vec2 p1, glm::vec2 p2, float thickness, const glm::vec4& color) {
		content.GetActiveLayer().AddShape(type, p1, p2, thickness, color);
		fileChanged = true;
	}

	void AddShapes(std::vector<ShapePTR>&& shapes) {
		content.GetActiveLayer().AddShapes(std::move(shapes));
		fileChanged = true;
	}

	bool RemoveShapes(const std::vector<ShapeID>& ids) {
		if (content.GetActiveLayer().RemoveShapes(ids)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesLeft(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesLeft(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesRight(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesRight(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesUp(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesUp(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapesDown(const std::vector<ShapeID>& ids, float amount) {
		if (content.GetActiveLayer().MoveShapesDown(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	bool MoveShapes(const std::vector<ShapeID>& ids, glm::vec2 amount) {
		if (content.GetActiveLayer().MoveShapes(ids, amount)) {
			fileChanged = true;
			return true;
		}
		return false;
	}

	std::optional<std::reference_wrapper<const GenericShape>> FindShape(const ShapeID& id) {
		return content.GetActiveLayer().FindShape(id);
	}

	void UpdateWindowTitle() {
		std::string file = Battery::FileUtils::GetBasenameFromPath(filename);
		if (fileChanged) {
			Battery::GetApplication()->window.SetTitle("*" + file + " - " APPLICATION_NAME);
		}
		else {
			Battery::GetApplication()->window.SetTitle(file + " - " APPLICATION_NAME);
		}
	}

	nlohmann::json GetJsonFromShapes(const std::vector<ShapeID>& ids) {
		nlohmann::json json = nlohmann::json();

		for (auto id : ids) {
			auto shape = FindShape(id);

			if (shape.has_value()) {
				json.push_back(shape.value().get().GetJson());
			}
		}

		return json;
	}

	bool ContainsChanges() {
		return fileChanged;
	}

	bool SaveFile(bool saveAs = false) {
		
		// First get the file content
		std::string content = GetJson().dump(4);
		std::string tempLocation = fileLocation;

		// Get file location if not known already
		if (tempLocation == "" || saveAs) {
			while (true) {
				tempLocation = Battery::FileUtils::PromptFileSaveDialog({ "*.*", "*.tsk" }, 
					Battery::Application::GetApplicationPointer()->window);

				// If location is still invalid, abort
				if (tempLocation == "") {
					return false;
				}

				// Append the extension
				if (Battery::FileUtils::GetExtensionFromPath(tempLocation) != ".tsk") {
					tempLocation += ".tsk";
				}

				// Warn and repeat if the file already exists
				if (Battery::FileUtils::FileExists(tempLocation)) {
					if (!Battery::ShowWarningMessageBoxYesNo("The file '" +
						Battery::FileUtils::GetFilenameFromPath(tempLocation) +
						"' already exists, are you sure you want to overwrite it?", 
						Battery::GetApplication()->window.allegroDisplayPointer))
					{
						continue;	// Repeat from top
					}
				}

				break;
			}
		}

		// Simply append the extension
		if (Battery::FileUtils::GetExtensionFromPath(tempLocation) != ".tsk") {
			tempLocation += ".tsk";
		}

		// Now save the file
		if (!Battery::FileUtils::WriteFile(tempLocation, content)) {
			Battery::ShowErrorMessageBox("The file '" + tempLocation + "' could not be saved!",
				Battery::GetApplication()->window.allegroDisplayPointer);
			return false;
		}

		// Saving was successful
		fileChanged = false;
		fileLocation = tempLocation;
		filename = Battery::FileUtils::GetFilenameFromPath(fileLocation);

		UpdateWindowTitle();

		return true;
	}

	bool OpenFile() {

		// First save the file
		if (ContainsChanges()) {

			bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
				"Do you want to save the file?", Battery::GetApplication()->window.allegroDisplayPointer);

			if (save) {	// File needs to be saved
				if (!SaveFile()) {	// Saving was not successful
					return false;
				}
			}
		}

		// Now open a new one
		std::string path = Battery::FileUtils::PromptFileOpenDialog({ "*.*", "*.tsk" },
			Battery::Application::GetApplicationPointer()->window);

		if (path == "") {
			return false;
		}

		return OpenFile(path);
	}

	bool OpenFile(const std::string& path) {

		if (Battery::FileUtils::GetExtensionFromPath(path) != ".tsk") {
			Battery::ShowErrorMessageBox("Can't load file '" + path + "': Unsupported file format, only .tsk files are supported",
				Battery::GetApplication()->window.allegroDisplayPointer);
			return false;
		}

		// First save the file
		if (ContainsChanges()) {

			bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
				"Do you want to save the file?", Battery::GetApplication()->window.allegroDisplayPointer);

			if (save) {	// File needs to be saved
				if (!SaveFile()) {	// Saving was not successful
					return false;
				}
			}
		}

		UpdateWindowTitle();

		// Now load the new file
		auto file = Battery::FileUtils::ReadFile(path);
		if (file.fail()) {
			Battery::ShowErrorMessageBox("Can't load file '" + path + "': File not found", 
				Battery::GetApplication()->window.allegroDisplayPointer);
			return false;
		}

		try {
			nlohmann::json j = nlohmann::json::parse(file.content());

			if (j["file_type"] != JSON_FILE_TYPE) {
				Battery::ShowErrorMessageBox("Can't load file: File type is unknown!", 
					Battery::GetApplication()->window.allegroDisplayPointer);
				return false;
			}

			if (j["file_version"] != JSON_FILE_VERSION) {
				Battery::ShowErrorMessageBox("Can't load file: File version is unsupported!", 
					Battery::GetApplication()->window.allegroDisplayPointer);
				return false;
			}

			// Now file seems to be loaded and supported, parse layers now
			std::vector<Layer> layers;
			for (nlohmann::json layerJson : j["layers"]) {
				Layer layer("");

				if (!layer.LoadJson(layerJson)) {
					Battery::ShowErrorMessageBox("Can't load file: File can't be parsed!",
						Battery::GetApplication()->window.allegroDisplayPointer);
					return false;
				}

				layers.push_back(std::move(layer));
			}

			// Json is parsed, now load the content
			this->content = FileContent();				// Clean everything
			for (Layer& layer : layers) {
				content.PushLayer(std::move(layer));
			}
			
			fileChanged = false;
			fileLocation = path;
			filename = Battery::FileUtils::GetFilenameFromPath(path);

			UpdateWindowTitle();

			return true;
		}
		catch (...) {
			Battery::ShowErrorMessageBox("Can't load file: JSON format is invalid!", 
				Battery::GetApplication()->window.allegroDisplayPointer);
		}

		return false;
	}

	nlohmann::json GetJson() {
		nlohmann::json j = nlohmann::json();

		nlohmann::json layers = nlohmann::json::array();
		for (Layer& layer : content.GetLayers()) {
			layers.push_back(layer.GetJson());
		}
		j["layers"] = layers;
		j["file_type"] = JSON_FILE_TYPE;
		j["file_version"] = JSON_FILE_VERSION;

		return j;
	}
};
