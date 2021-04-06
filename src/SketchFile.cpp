
#include "pch.h"
#include "SketchFile.h"
#include "Navigator.h"

void SketchFile::UpdateWindowTitle() {
	std::string file = Battery::FileUtils::GetBasenameFromPath(filename);
	std::string version = Navigator::GetInstance()->GetApplicationVersion();

	if (fileChanged) {
		Battery::GetApplication()->window.SetTitle("*" + file + " - " APPLICATION_NAME + " - " + version);
	}
	else {
		Battery::GetApplication()->window.SetTitle(file + " - " APPLICATION_NAME + " - " + version);
	}
}

bool SketchFile::SaveFile(bool saveAs) {

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
	Navigator::GetInstance()->AppendRecentFile(fileLocation);

	return true;
}

bool SketchFile::OpenFile() {

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

	// Now open a new one
	std::string path = Battery::FileUtils::PromptFileOpenDialog({ "*.*", "*.tsk" },
		Battery::Application::GetApplicationPointer()->window);

	if (path == "") {
		return false;
	}

	return OpenFile(path);
}

bool SketchFile::OpenFile(const std::string& path) {

	if (Battery::FileUtils::GetExtensionFromPath(path) != ".tsk") {
		Battery::ShowErrorMessageBox("Can't load file '" + path + "': Unsupported file format, only .tsk files are supported",
			Battery::GetApplication()->window.allegroDisplayPointer);
		return false;
	}

	UpdateWindowTitle();

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
		this->content = FileContent(false);				// Clean everything
		for (Layer& layer : layers) {
			content.PushLayer(std::move(layer));
		}

		fileChanged = false;
		fileLocation = path;
		filename = Battery::FileUtils::GetFilenameFromPath(path);

		UpdateWindowTitle();
		Navigator::GetInstance()->AppendRecentFile(fileLocation);

		return true;
	}
	catch (...) {
		Battery::ShowErrorMessageBox("Can't load file: JSON format is invalid!",
			Battery::GetApplication()->window.allegroDisplayPointer);
	}

	return false;
}
