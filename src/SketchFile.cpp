
#include "battery/core/all.hpp"
#include "battery/filedialog/filedialog.hpp"
#include "App.hpp"
#include "SketchFile.h"
#include "TskSettings.hpp"
#include "Navigator.h"

void SketchFile::UpdateWindowTitle() {
	auto file = m_filename.stem().string();

	b::string title = b::format("{}{} - {}", m_fileChanged ? "*" : "", file, APPLICATION_NAME);

    auto version = Navigator::GetInstance()->m_applicationVersion;
	if (!version.to_string().empty()) {
		title += b::format(" - {}", version.to_string());
	}

	App::s_mainWindow->setTitle(title);
}

bool SketchFile::SaveFile(bool saveAs) {

	// First get the file content
	auto content = GetJson().dump(4);
	auto tempLocation = m_filepath;

	// Get file location if not known already
	if (tempLocation == "" || saveAs) {
		while (true) {
//			tempLocation = b::PromptFileSaveDialog({ "*.*", "*.tsk" }, window.getSystemHandle());
            b::log::critical("PROMPTFILESAVEDIALOG NOW");

			// If location is still invalid, abort
			if (tempLocation == "") {
				return false;
			}

			// Append the extension
			if (tempLocation.extension().u8string() != u8".tsk") {
				tempLocation += ".tsk";
			}

			// Warn and repeat if the file already exists
//			if (b::fs::exists(tempLocation)) {
//				if (!b::message_box_warning(
//                        fmt::format("The file '{}' already exists, are you sure you want to overwrite it?",
//                                    b::u8_as_str(tempLocation.filename().u8string()))))
//				{
//					continue;	// Repeat from top
//				}
//			}
			b::log::error("SHOWWARNINGMESSAGEBOX WITH RETURN VALUE NOW");

			break;
		}
	}

	// Simply append the extension
	if (tempLocation.extension().u8string() != u8".tsk") {
		tempLocation += ".tsk";
	}

	sf::Cursor cursor;
	(void)cursor.loadFromSystem(sf::Cursor::ArrowWait);
	App::s_mainWindow->setMouseCursor(cursor);

	// Now save the file
	if (!b::fs::write_text_file_nothrow(tempLocation, content)) {
        (void)cursor.loadFromSystem(sf::Cursor::Arrow);
		App::s_mainWindow->setMouseCursor(cursor);
		b::message_box_warning(b::format("The file '{}' could not be saved!", tempLocation));
		return false;
	}

	// Saving was successful
	m_fileChanged = false;
	m_filepath = tempLocation;
	m_filename = tempLocation.filename();

	UpdateWindowTitle();
	TskSettings::AddRecentFile(m_filepath);

	(void)cursor.loadFromSystem(sf::Cursor::Arrow);
    App::s_mainWindow->setMouseCursor(cursor);

	return true;
}

bool SketchFile::OpenFile() {

	// Now open a new one
    b::filedialog dialog;
    // TODO: Implement proper file dialog
	std::string path = dialog.sync_open();

	if (path.empty()) {
		return false;
	}

	return OpenFile(path);
}

bool SketchFile::OpenEmptyFile() {

	// First save the file
	/*if (ContainsChanges()) {
		bool save = b::message_box_warning("This file contains unsaved changes! "
			"Do you want to save the file?", Battery::GetMainWindow().allegroDisplayPointer);

		if (save) {	// File needs to be saved
			if (!SaveFile()) {	// Saving was not successful
				return false;
			}
		}
	}*/
    b::log::error("SHOWWARNINGMESSAGEBOX WITH RETURN VALUE NOW");

	UpdateWindowTitle();

	// Now the file is saved and can be emptied
	content = FileContent();

	m_fileChanged = false;
	m_filename = DEFAULT_FILENAME;	// Filename contains extension
	m_filepath = "";
	canvasColor = DEFAULT_BACKGROUND_COLOR;

	Navigator::GetInstance()->ResetViewport();

	return true;
}

bool SketchFile::OpenFile(const b::fs::path& path, bool silent) {

//	auto& window = Battery::GetApp().window;

	/*if (Battery::GetExtension(path) != ".tsk") {
		if (!silent) {
			Battery::ShowErrorMessageBox("Can't load file '" + path + "': Unsupported file format, only .tsk files are supported",
				window.allegroDisplayPointer);
		}
		return false;
	}*/

	UpdateWindowTitle();

	// First save the file
	/*if (ContainsChanges()) {

		bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
			"Do you want to save the file?", window.allegroDisplayPointer);

		if (save) {	// File needs to be saved
			if (!SaveFile()) {	// Saving was not successful
				return false;
			}
		}
	}*/

	UpdateWindowTitle();

	/*window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_BUSY);

	// Now load the new file
	auto file = Battery::ReadFile(path);
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
		ImVec4 bCol;
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
		canvasColor = bCol;
		filename = Battery::GetFilename(path);

		UpdateWindowTitle();
		Navigator::GetInstance()->AppendRecentFile(fileLocation);
		window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);

		Navigator::GetInstance()->ResetViewport();

		return true;
	}
	catch (...) {
		window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
		if (!silent) {
			Battery::ShowErrorMessageBox("Can't load file: JSON format is invalid!",
				window.allegroDisplayPointer);
		}
	}

	window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);*/
	return false;
}

sf::Image SketchFile::ExportImage(bool transparent, float dpi) {

	// Calculate the bounding box
	ImVec2 min = GetActiveLayer().GetBoundingBox().first;
	ImVec2 max = GetActiveLayer().GetBoundingBox().second;
	for (auto& layer : GetLayers()) {
		auto bound = layer.GetBoundingBox();

		min = b::min(min, bound.first);
		max = b::max(max, bound.second);
	}

	// Apply a small margin around the image (% of the entire image)
	float margin = 0.08f;
	float sizeX = abs(max.x - min.x);
	float sizeY = abs(max.y - min.y);
	min -= ImVec2(sizeX, sizeY) * margin;
	max += ImVec2(sizeX, sizeY) * margin;

	// Calculate image size
	float dpmm = dpi / 25.4;	// Convert dots per inch to dots per mm
	float width = sizeX * dpmm;
	float height = width / sizeX * sizeY;

	if (width <= 0.0 || height <= 0.0 || isnan(width) || isnan(height))
		return {};

//	// Initialize texture image to render on
//	Battery::Bitmap image(width, height);
//	std::unique_ptr<Battery::Scene> scene = std::make_unique<Battery::Scene>(Battery::GetMainWindow(), image);
//	//b::log::warn("Created Battery::Texture2D");
//
//	// Initialize the renderer
//	Battery::Renderer2D::BeginScene(scene.get());
//
//	if (transparent) {
//		al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
//		Battery::Renderer2D::DrawBackground(COLOR_TRANSPARENT);
//		//b::log::warn("Transparent mode");
//	}
//	else {
//		Battery::Renderer2D::DrawBackground(EXPORT_BACKGROUND_COLOR);
//		//Battery::Renderer2D::DrawBackground(ImVec4(255, 0, 255, 255));
//		//b::log::warn("Non-transparent mode");
//	}
//
//	// Render layers in reverse order
//	auto& layers = GetLayers();
//	for (size_t layerIndex = layers.size() - 1; layerIndex < layers.size(); layerIndex--) {
//		auto& layer = layers[layerIndex];
//
//		for (auto& shape : layer.GetShapes()) {
//
//			// Render the shape
//			shape->RenderExport(min, max, width, height);
//			//b::log::error("Rendering shape #{}", shape->GetID());
//		}
//	}
//
//	Battery::Renderer2D::EndScene();
//	//b::log::warn("Export finished");

    b::log::critical("EXPORT NOW");
	return {};
}
