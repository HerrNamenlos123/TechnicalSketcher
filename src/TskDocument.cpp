
#include "TskDocument.hpp"

bool TskDocument::Layer::containsUnsavedChanges() const {
    return m_containsUnsavedChanges;
}

void TskDocument::addLayer(b::string layerName) {
    if (layerName.empty()) {     // Auto-generate a unique name
        size_t id = 1;
        do {
            layerName = b::format("Layer {}", id++);
        } while (layerExists(layerName));
    }
    m_layers.emplace_back(layerName);
    m_activeLayerIndex = std::clamp<size_t>(m_activeLayerIndex, 0, m_layers.size() - 1);
    m_containsUnsavedChanges = true;
}

void TskDocument::removeLayer(size_t index) {
    b::erase_nth_element(m_layers, index);
    if (m_layers.empty()) {
        addLayer();
    }
    m_activeLayerIndex = std::clamp<size_t>(m_activeLayerIndex, 0, m_layers.size() - 1);
    m_containsUnsavedChanges = true;
}

void TskDocument::removeLayer(const LayerID& id) {
    auto it = std::find_if(m_layers.begin(), m_layers.end(), [&id](const TskDocument::Layer& layer) { return layer.id() == id; });
    auto index = std::distance(m_layers.begin(), it);
    removeLayer(index);
}

bool TskDocument::layerExists(const b::string& name) const {
    return std::find_if(m_layers.begin(), m_layers.end(), [&name](const TskDocument::Layer& layer) { return layer.name() == name; }) != m_layers.end();
}

const TskDocument::Layer& TskDocument::getLayer(const LayerID& id) const {
    return *std::find_if(m_layers.begin(), m_layers.end(), [&id](const auto& layer) { return layer.id() == id; });
}

size_t TskDocument::getLayerIndex(const LayerID& id) const {
    auto it = std::find_if(m_layers.begin(), m_layers.end(), [&id](const auto& layer) { return layer.id() == id; });
    return std::distance(m_layers.begin(), it);
}

const std::vector<TskDocument::Layer>& TskDocument::getLayers() const {
    return m_layers;
}


//
//#include "battery/core/all.hpp"
//#include "battery/filedialog/filedialog.hpp"
//#include "App.hpp"
//#include "SketchFile.h"
//#include "TskSettings.hpp"
//#include "Navigator.h"
//
//void SketchFile::UpdateWindowTitle() {
//	auto file = m_filename.stem().string();
//
//	b::string title = b::format("{}{} - {}", m_fileChanged ? "*" : "", file, APPLICATION_NAME);
//
//    auto version = Navigator::GetInstance()->m_applicationVersion;
//	if (!version.to_string().empty()) {
//		title += b::format(" - {}", version.to_string());
//	}
//
//	App::s_mainWindow->setTitle(title);
//}
//
//bool SketchFile::SaveFile(bool saveAs) {
//
//	// First get the file content
//	auto content = GetJson().dump(4);
//	auto tempLocation = m_filepath;
//
//	// Get file location if not known already
//	if (tempLocation == "" || saveAs) {
//		while (true) {
////			tempLocation = b::PromptFileSaveDialog({ "*.*", "*.tsk" }, window.getSystemHandle());
//            b::log::critical("PROMPTFILESAVEDIALOG NOW");
//
//			// If location is still invalid, abort
//			if (tempLocation == "") {
//				return false;
//			}
//
//			// Append the extension
//			if (tempLocation.extension().u8string() != u8".tsk") {
//				tempLocation += ".tsk";
//			}
//
//			// Warn and repeat if the file already exists
////			if (b::fs::exists(tempLocation)) {
////				if (!b::message_box_warning(
////                        fmt::format("The file '{}' already exists, are you sure you want to overwrite it?",
////                                    b::u8_as_str(tempLocation.filename().u8string()))))
////				{
////					continue;	// Repeat from top
////				}
////			}
//			b::log::error("SHOWWARNINGMESSAGEBOX WITH RETURN VALUE NOW");
//
//			break;
//		}
//	}
//
//	// Simply append the extension
//	if (tempLocation.extension().u8string() != u8".tsk") {
//		tempLocation += ".tsk";
//	}
//
//	sf::Cursor cursor;
//	(void)cursor.loadFromSystem(sf::Cursor::ArrowWait);
//	App::s_mainWindow->setMouseCursor(cursor);
//
//	// Now save the file
//	if (!b::fs::write_text_file_nothrow(tempLocation, content)) {
//        (void)cursor.loadFromSystem(sf::Cursor::Arrow);
//		App::s_mainWindow->setMouseCursor(cursor);
//		b::message_box_warning(b::format("The file '{}' could not be saved!", tempLocation));
//		return false;
//	}
//
//	// Saving was successful
//	m_fileChanged = false;
//	m_filepath = tempLocation;
//	m_filename = tempLocation.filename();
//
//	UpdateWindowTitle();
//	TskSettings::AddRecentFile(m_filepath);
//
//	(void)cursor.loadFromSystem(sf::Cursor::Arrow);
//    App::s_mainWindow->setMouseCursor(cursor);
//
//	return true;
//}
//
//bool SketchFile::OpenFile() {
//
//	// Now open a new one
//    b::filedialog dialog;
//    // TODO: Implement proper file dialog
//	std::string path = dialog.sync_open();
//
//	if (path.empty()) {
//		return false;
//	}
//
//	return OpenFile(path);
//}
//
//bool SketchFile::OpenEmptyFile() {
//
//	// First save the file
//	/*if (ContainsChanges()) {
//		bool save = b::message_box_warning("This file contains unsaved changes! "
//			"Do you want to save the file?", Battery::GetMainWindow().allegroDisplayPointer);
//
//		if (save) {	// File needs to be saved
//			if (!SaveFile()) {	// Saving was not successful
//				return false;
//			}
//		}
//	}*/
//    b::log::error("SHOWWARNINGMESSAGEBOX WITH RETURN VALUE NOW");
//
//	UpdateWindowTitle();
//
//	// Now the file is saved and can be emptied
//	content = FileContent();
//
//	m_fileChanged = false;
//	m_filename = DEFAULT_FILENAME;	// Filename contains extension
//	m_filepath = "";
//	canvasColor = DEFAULT_BACKGROUND_COLOR;
//
//	Navigator::GetInstance()->ResetViewport();
//
//	return true;
//}
//
//bool SketchFile::OpenFile(const b::fs::path& path, bool silent) {
//
////	auto& window = Battery::GetApp().window;
//
//	/*if (Battery::GetExtension(path) != ".tsk") {
//		if (!silent) {
//			Battery::ShowErrorMessageBox("Can't load file '" + path + "': Unsupported file format, only .tsk files are supported",
//				window.allegroDisplayPointer);
//		}
//		return false;
//	}*/
//
//	UpdateWindowTitle();
//
//	// First save the file
//	/*if (ContainsChanges()) {
//
//		bool save = Battery::ShowWarningMessageBoxYesNo("This file contains unsaved changes! "
//			"Do you want to save the file?", window.allegroDisplayPointer);
//
//		if (save) {	// File needs to be saved
//			if (!SaveFile()) {	// Saving was not successful
//				return false;
//			}
//		}
//	}*/
//
//	UpdateWindowTitle();
//
//	/*window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_BUSY);
//
//	// Now load the new file
//	auto file = Battery::ReadFile(path);
//	if (file.fail()) {
//		window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
//		if (!silent) {
//			Battery::ShowErrorMessageBox("Can't load file '" + path + "': File not found",
//				window.allegroDisplayPointer);
//		}
//		return false;
//	}
//
//	// Now it's valid, check if it is empty
//	if (file.content().length() == 0) {
//		// Create an empty file
//		return OpenEmptyFile();
//	}
//
//	try {
//		nlohmann::json j = nlohmann::json::parse(file.content());
//
//		if (j["file_type"] != JSON_FILE_TYPE) {
//			window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
//			if (!silent) {
//				Battery::ShowErrorMessageBox("Can't load file: File type is unknown!",
//					window.allegroDisplayPointer);
//			}
//			return false;
//		}
//
//		if (j["file_version"] != JSON_FILE_VERSION) {
//			window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
//			if (!silent) {
//				Battery::ShowErrorMessageBox("Can't load file: File version is unsupported!",
//					window.allegroDisplayPointer);
//			}
//			return false;
//		}
//
//		// Now file seems to be loaded and supported, parse layers now
//		std::vector<Layer> layers;
//		for (nlohmann::json layerJson : j["layers"]) {
//			Layer layer("");
//
//			if (!layer.LoadJson(layerJson)) {
//				window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
//				if (!silent) {
//					Battery::ShowErrorMessageBox("Can't load file: File can't be parsed!",
//						window.allegroDisplayPointer);
//				}
//				return false;
//			}
//
//			layers.push_back(std::move(layer));
//		}
//
//		// And finally, load the background color
//		ImVec4 bCol;
//		if (j.contains("background_color")) {
//			std::vector<float> col = j["background_color"];
//
//			if (col.size() == 4) {
//				bCol.r = col[0];
//				bCol.g = col[1];
//				bCol.b = col[2];
//				bCol.a = col[3];
//			}
//			else {
//				bCol = DEFAULT_BACKGROUND_COLOR;
//			}
//		}
//		else {
//			bCol = DEFAULT_BACKGROUND_COLOR;
//		}
//
//		// Json is parsed, now load the content
//		this->content = FileContent(false);				// Clean everything
//		for (Layer& layer : layers) {
//			content.PushLayer(std::move(layer));
//		}
//
//		fileChanged = false;
//		fileLocation = path;
//		canvasColor = bCol;
//		filename = Battery::GetFilename(path);
//
//		UpdateWindowTitle();
//		Navigator::GetInstance()->AppendRecentFile(fileLocation);
//		window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
//
//		Navigator::GetInstance()->ResetViewport();
//
//		return true;
//	}
//	catch (...) {
//		window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
//		if (!silent) {
//			Battery::ShowErrorMessageBox("Can't load file: JSON format is invalid!",
//				window.allegroDisplayPointer);
//		}
//	}
//
//	window.SetMouseCursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);*/
//	return false;
//}
//
//sf::Image SketchFile::ExportImage(bool transparent, float dpi) {
//
//	// Calculate the bounding box
//	ImVec2 min = GetActiveLayer().GetBoundingBox().first;
//	ImVec2 max = GetActiveLayer().GetBoundingBox().second;
//	for (auto& layer : GetLayers()) {
//		auto bound = layer.GetBoundingBox();
//
//		min = b::min(min, bound.first);
//		max = b::max(max, bound.second);
//	}
//
//	// Apply a small margin around the image (% of the entire image)
//	float margin = 0.08f;
//	float sizeX = abs(max.x - min.x);
//	float sizeY = abs(max.y - min.y);
//	min -= ImVec2(sizeX, sizeY) * margin;
//	max += ImVec2(sizeX, sizeY) * margin;
//
//	// Calculate image size
//	float dpmm = dpi / 25.4;	// Convert dots per inch to dots per mm
//	float width = sizeX * dpmm;
//	float height = width / sizeX * sizeY;
//
//	if (width <= 0.0 || height <= 0.0 || isnan(width) || isnan(height))
//		return {};
//
////	// Initialize texture image to render on
////	Battery::Bitmap image(width, height);
////	std::unique_ptr<Battery::Scene> scene = std::make_unique<Battery::Scene>(Battery::GetMainWindow(), image);
////	//b::log::warn("Created Battery::Texture2D");
////
////	// Initialize the renderer
////	Battery::Renderer2D::BeginScene(scene.get());
////
////	if (transparent) {
////		al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
////		Battery::Renderer2D::DrawBackground(COLOR_TRANSPARENT);
////		//b::log::warn("Transparent mode");
////	}
////	else {
////		Battery::Renderer2D::DrawBackground(EXPORT_BACKGROUND_COLOR);
////		//Battery::Renderer2D::DrawBackground(ImVec4(255, 0, 255, 255));
////		//b::log::warn("Non-transparent mode");
////	}
////
////	// Render layers in reverse order
////	auto& layers = GetLayers();
////	for (size_t layerIndex = layers.size() - 1; layerIndex < layers.size(); layerIndex--) {
////		auto& layer = layers[layerIndex];
////
////		for (auto& shape : layer.GetShapes()) {
////
////			// Render the shape
////			shape->RenderExport(min, max, width, height);
////			//b::log::error("Rendering shape #{}", shape->GetID());
////		}
////	}
////
////	Battery::Renderer2D::EndScene();
////	//b::log::warn("Export finished");
//
//    b::log::critical("EXPORT NOW");
//	return {};
//}

bool TskDocument::containsUnsavedChanges() const {
    if (m_containsUnsavedChanges) { // <- The document itself or layers were changed
        return true;
    }                               // vv Something in one of the layers was changed
    if (std::ranges::any_of(m_layers, [](const auto& layer) { return layer.containsUnsavedChanges(); })) {
        return true;
    }
    return false;
}

void TskDocument::centerOnCanvas(b::Canvas& canvas) {
    b::Vec2 diff = canvas.mapPixelToCoords({0, 0 }, m_cameraView) -
                   canvas.mapPixelToCoords(b::Vec2(canvas.getSize()), m_cameraView);
    m_cameraView.setSize(b::Vec2(canvas.getSize()));
    m_cameraView.move(diff / 2.0);
    m_cameraView.zoom(0.1f);
}

void TskDocument::renderToCanvas(b::Canvas& canvas) {
    canvas.clear(m_canvasColor);
    renderGrid(canvas);

    if (canvas.mouse.leftButtonPressed) {
        auto deltaUnits = canvas.mapPixelToCoords({ 0, 0 }, m_cameraView) -
                          canvas.mapPixelToCoords(canvas.mouse.posDelta * b::Vec2(1, -1), m_cameraView);
        m_cameraView.move(deltaUnits);
    }

    if (canvas.mouse.scrollDelta.length() != 0) {
        auto mouseToCenter = b::Vec2(m_cameraView.getCenter()) - canvas.mapPixelToCoords(canvas.mouse.pos, m_cameraView);
        double zoomFactor = 1.0 - (canvas.mouse.scrollDelta.x + canvas.mouse.scrollDelta.y) * 0.1;
        m_cameraView.zoom((float) zoomFactor);
        m_cameraView.move((mouseToCenter * zoomFactor - mouseToCenter) * b::Vec2(1, -1));
    }

    if (m_firstRenderPass) {        // Do this here because the window size might not be known earlier
        centerOnCanvas(canvas);
        m_firstRenderPass = false;
    }

    renderGrid(canvas);
    renderLayers(canvas);
}

void TskDocument::renderGrid(b::Canvas& canvas) {
    canvas.setView(sf::View(b::Vec2(canvas.getSize()) / 2.0, b::Vec2(canvas.getSize())));

    b::Color gridLineColor = TskSettings::Get(TskSetting::DOCUMENT_GRID_LINE_COLOR);
    double gridLineWidth = TskSettings::Get(TskSetting::DOCUMENT_GRID_LINE_WIDTH);
    auto leftUpperMostPixelCoords = canvas.mapPixelToCoords({ 0, 0 }, m_cameraView);

    double unit = std::floor(leftUpperMostPixelCoords.x / m_gridDecadeFactor) * m_gridDecadeFactor;
    double x = 0;
    while (x < canvas.getSize().x) {
        x = canvas.mapCoordsToPixel({static_cast<float>(unit), 0 }, m_cameraView).x;
        m_batchRenderer.drawLine({ x, 0 }, { x, b::Vec2(canvas.getSize()).y }, gridLineColor, gridLineWidth);
        unit += m_gridDecadeFactor;
    }

    unit = std::floor(leftUpperMostPixelCoords.y / m_gridDecadeFactor) * m_gridDecadeFactor;
    double y = 0;
    while (y < canvas.getSize().y) {
        y = canvas.mapCoordsToPixel({0, static_cast<float>(unit) }, m_cameraView).y;
        m_batchRenderer.drawLine({ 0, y }, {b::Vec2(canvas.getSize()).x, y }, gridLineColor, gridLineWidth);
        unit += m_gridDecadeFactor;
    }

    canvas.draw(m_batchRenderer);
    m_batchRenderer.clear();
}

void TskDocument::renderLayers(b::Canvas& canvas) {
    canvas.setView(m_cameraView);
    for (auto& layer : m_layers) {
//        layer.render(canvas);
    }

    m_batchRenderer.drawRect({ -5, -5 }, { 5, 5 }, sf::Color::Red, 0.1f);
    canvas.draw(m_batchRenderer);
    m_batchRenderer.clear();
}
