
#include "pch.h"
#include "SketchFile.h"
#include "Navigator.h"

#undef min
#undef max

void SketchFile::UpdateWindowTitle() {
	std::string file = Battery::FileUtils::GetBasenameFromPath(filename);
	const std::string& version = Navigator::GetInstance()->applicationVersion;
	std::string title = "";

	if (fileChanged) {
		title = "*" + file + " - " APPLICATION_NAME;
	}
	else {
		title = file + " - " APPLICATION_NAME;
	}

	if (version.length() > 0) {
		title += " - " + version;
	}

	Battery::GetMainWindow().SetTitle(title);
}

bool SketchFile::SaveFile(bool saveAs) {

	auto& window = Battery::GetMainWindow();

	// First get the file content
	std::string content = GetJson().dump(4);
	std::string tempLocation = fileLocation;

	// Get file location if not known already
	if (tempLocation == "" || saveAs) {
		while (true) {
			tempLocation = Battery::FileUtils::PromptFileSaveDialog({ "*.*", "*.tsk" }, window);

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
					window.allegroDisplayPointer))
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

	window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_BUSY);

	// Now save the file
	if (!Battery::FileUtils::WriteFile(tempLocation, content)) {
		window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
		Battery::ShowErrorMessageBox("The file '" + tempLocation + "' could not be saved!",
			window.allegroDisplayPointer);
		return false;
	}

	// Saving was successful
	fileChanged = false;
	fileLocation = tempLocation;
	filename = Battery::FileUtils::GetFilenameFromPath(fileLocation);

	UpdateWindowTitle();
	Navigator::GetInstance()->AppendRecentFile(fileLocation);
	Battery::GetMainWindow().SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);

	return true;
}

bool SketchFile::OpenFile() {

	// First save the file
	if (ContainsChanges()) {

		bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
			"Do you want to save the file?", Battery::GetMainWindow().allegroDisplayPointer);

		if (save) {	// File needs to be saved
			if (!SaveFile()) {	// Saving was not successful
				return false;
			}
		}
	}

	UpdateWindowTitle();

	// Now open a new one
	std::string path = Battery::FileUtils::PromptFileOpenDialog({ "*.*", "*.tsk" }, Battery::GetMainWindow());

	if (path == "") {
		return false;
	}

	return OpenFile(path);
}

bool SketchFile::OpenEmptyFile() {

	// First save the file
	if (ContainsChanges()) {

		bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
			"Do you want to save the file?", Battery::GetMainWindow().allegroDisplayPointer);

		if (save) {	// File needs to be saved
			if (!SaveFile()) {	// Saving was not successful
				return false;
			}
		}
	}

	UpdateWindowTitle();

	// Now the file is saved and can be emptied
	content = FileContent();

	fileChanged = false;
	filename = DEFAULT_FILENAME;	// Filename contains extension
	fileLocation = "";
	backgroundColor = DEFAULT_BACKGROUND_COLOR;
	return true;
}

bool SketchFile::OpenFile(const std::string& path, bool silent) {

	auto& window = Battery::GetMainWindow();

	if (Battery::FileUtils::GetExtensionFromPath(path) != ".tsk") {
		if (!silent) {
			Battery::ShowErrorMessageBox("Can't load file '" + path + "': Unsupported file format, only .tsk files are supported",
				window.allegroDisplayPointer);
		}
		return false;
	}

	UpdateWindowTitle();

	// First save the file
	if (ContainsChanges()) {

		bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
			"Do you want to save the file?", window.allegroDisplayPointer);

		if (save) {	// File needs to be saved
			if (!SaveFile()) {	// Saving was not successful
				return false;
			}
		}
	}

	UpdateWindowTitle();

	window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_BUSY);

	// Now load the new file
	auto file = Battery::FileUtils::ReadFile(path);
	if (file.fail()) {
		window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
		if (!silent) {
			Battery::ShowErrorMessageBox("Can't load file '" + path + "': File not found",
				window.allegroDisplayPointer);
		}
		return false;
	}

	// Now it's valid, check if it is empty
	if (file.content().length() == 0) {
		// Create an empty file
		return OpenEmptyFile();
	}

	try {
		nlohmann::json j = nlohmann::json::parse(file.content());

		if (j["file_type"] != JSON_FILE_TYPE) {
			window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
			if (!silent) {
				Battery::ShowErrorMessageBox("Can't load file: File type is unknown!",
					window.allegroDisplayPointer);
			}
			return false;
		}

		if (j["file_version"] != JSON_FILE_VERSION) {
			window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
			if (!silent) {
				Battery::ShowErrorMessageBox("Can't load file: File version is unsupported!",
					window.allegroDisplayPointer);
			}
			return false;
		}

		// Now file seems to be loaded and supported, parse layers now
		std::vector<Layer> layers;
		for (nlohmann::json layerJson : j["layers"]) {
			Layer layer("");

			if (!layer.LoadJson(layerJson)) {
				window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
				if (!silent) {
					Battery::ShowErrorMessageBox("Can't load file: File can't be parsed!",
						window.allegroDisplayPointer);
				}
				return false;
			}

			layers.push_back(std::move(layer));
		}

		// And finally, load the background color
		glm::vec4 bCol;
		if (j.contains("background_color")) {
			std::vector<float> col = j["background_color"];

			if (col.size() == 4) {
				bCol.r = col[0];
				bCol.g = col[1];
				bCol.b = col[2];
				bCol.a = col[3];
			}
			else {
				bCol = DEFAULT_BACKGROUND_COLOR;
			}
		}
		else {
			bCol = DEFAULT_BACKGROUND_COLOR;
		}

		// Json is parsed, now load the content
		this->content = FileContent(false);				// Clean everything
		for (Layer& layer : layers) {
			content.PushLayer(std::move(layer));
		}

		fileChanged = false;
		fileLocation = path;
		backgroundColor = bCol;
		filename = Battery::FileUtils::GetFilenameFromPath(path);

		UpdateWindowTitle();
		Navigator::GetInstance()->AppendRecentFile(fileLocation);
		window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);

		return true;
	}
	catch (...) {
		window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
		if (!silent) {
			Battery::ShowErrorMessageBox("Can't load file: JSON format is invalid!",
				window.allegroDisplayPointer);
		}
	}

	window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
	return false;
}

Battery::Texture2D SketchFile::ExportImage(bool transparent, float dpi) {

	// Calculate the bounding box
	glm::vec2 min = GetActiveLayer().GetBoundingBox().first;
	glm::vec2 max = GetActiveLayer().GetBoundingBox().second;
	for (auto& layer : GetLayers()) {
		auto bound = layer.GetBoundingBox();

		min = glm::min(min, bound.first);
		max = glm::max(max, bound.second);
	}

	// Apply a small margin around the image (% of the entire image)
	float margin = 0.08;
	float sizeX = abs(max.x - min.x);
	float sizeY = abs(max.y - min.y);
	min -= glm::vec2(sizeX, sizeY) * margin;
	max += glm::vec2(sizeX, sizeY) * margin;

	// Calculate image size
	float dpmm = dpi / 25.4;	// Convert dots per inch to dots per mm
	float width = sizeX * dpmm;
	float height = width / sizeX * sizeY;

	// Initialize texture image to render on
	Battery::Texture2D image(width, height);
	std::unique_ptr<Battery::Scene> scene = std::make_unique<Battery::Scene>(Battery::GetMainWindow(), image);
	//LOG_WARN("Created Battery::Texture2D");

	// Initialize the renderer
	Battery::Renderer2D::BeginScene(scene.get());
	
	if (transparent) {
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
		Battery::Renderer2D::DrawBackground(COLOR_TRANSPARENT);
		//LOG_WARN("Transparent mode");
	}
	else {
		Battery::Renderer2D::DrawBackground(EXPORT_BACKGROUND_COLOR);
		//Battery::Renderer2D::DrawBackground(glm::vec4(255, 0, 255, 255));
		//LOG_WARN("Non-transparent mode");
	}

	// Render layers in reverse order
	auto& layers = GetLayers();
	for (size_t layerIndex = layers.size() - 1; layerIndex < layers.size(); layerIndex--) {
		auto& layer = layers[layerIndex];

		for (auto& shape : layer.GetShapes()) {

			// Render the shape
			shape->RenderExport(min, max, width, height);
			//LOG_ERROR("Rendering shape #{}", shape->GetID());
		}
	}

	Battery::Renderer2D::EndScene();
	//LOG_WARN("Export finished");
	return image;
}
