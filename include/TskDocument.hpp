#pragma once

#include "pch.hpp"
#include "TskSettings.hpp"
//
//class DocumentLayers {
//
//    std::vector<SketchLayer> layers;
//    size_t activeLayerIndex = -1;
//
//public:
//    FileContent(bool init = true) {
//        if (init) {
//            PushLayer();
//            p_CorrectLayers();
//        }
//    }
//
//    SketchLayer& GetActiveLayer() {
//        p_CorrectLayers();
//        return layers[activeLayerIndex];
//    }
//
//    std::vector<SketchLayer>& GetLayers() {
//        p_CorrectLayers();
//        return layers;
//    }
//
//    const std::vector<ShapePTR>& GetActiveLayerShapes() {
//        return GetActiveLayer().GetShapes();
//    }
//
//    bool ActivateLayer(LayerID id) {
//
//        for (size_t i = 0; i < layers.size(); i++) {
//            if (layers[i].GetID() == id) {
//                activeLayerIndex = i;
//                return true;
//            }
//        }
//
//        b::log::warn(__FUNCTION__"(): Can't activate Layer #{}: Does not exist!", id);
//        return false;
//    }
//
//    void GeneratePreviews() {
//        //for (auto& layer : layers) {
//        //	layer.GeneratePreview();
//        //}
//    }
//
//public:
//    // Correct activeLayerIndex, if it's -1 or too large
//    void p_CorrectLayers() {
//
//        if (layers.size() == 0) {
//            PushLayer();
//        }
//
//        if (activeLayerIndex == -1) {
//            activeLayerIndex = 0;
//        }
//        else if (activeLayerIndex >= layers.size()) {
//            activeLayerIndex = layers.size() - 1;
//        }
//    }
//
//    // Find the std::vector index of the layer with the specified id: -1 if not found
//    size_t p_FindLayerIndex(ShapeID id) {
//
//        for (size_t i = 0; i < layers.size(); i++) {
//            if (layers[i].GetID() == id) {
//                return i;
//            }
//        }
//
//        return -1;
//    }
//
//    bool p_IsIndexValid(size_t index) {
//        return index < layers.size();
//    }
//};

class LayerID {
public:
    LayerID& operator=(const LayerID& other) = delete;
    LayerID& operator=(LayerID&& other) noexcept = default;

    LayerID(const LayerID& other) = delete;
    LayerID(LayerID&& other) noexcept = default;

    inline static LayerID New() {
        LayerID id;
        id.m_uuid = b::uuid::v4();
        return id;
    }

    inline bool operator==(const LayerID& other) const {
        return m_uuid == other.m_uuid;
    }

    inline bool operator!=(const LayerID& other) const {
        return m_uuid != other.m_uuid;
    }

private:
    LayerID() = default;

    b::string m_uuid;
};



/// This class guarantees that always at least 1 layer exists
class TskDocument {
public:
    class Layer {
    public:
        Layer(const b::string& name) : m_id(LayerID::New()), m_name(name) {}

//        LayerState GetState();
//        void LoadState(const LayerState& state);
//        DocumentLayer Duplicate();

        const LayerID& id() const { return m_id; }
        const b::string& name() const { return m_name; }

//        const auto& getShapes() const { return m_shapesHistory[0]; }

//        bool addShape(const nlohmann::json& json);
//        void addShape(enum class ShapeType type, ImVec2 p1, ImVec2 p2, float thickness, const ImVec4& color);
//        void addShape(enum class ShapeType type, ImVec2 center, float radius, float thickness, const ImVec4& color);
//        void addShape(enum class ShapeType type, ImVec2 center, float radius, float startAngle, float endAngle, float thickness, const ImVec4& color);
//        bool addShapes(const std::vector<nlohmann::json>& jsonArray);
//        void addShapes(std::vector<ShapePTR>&& shapes);
//        bool RemoveShape(const ShapeID& id);
//        bool RemoveShapes(const std::vector<ShapeID>& ids);
//
//        bool MoveShapeLeft(const ShapeID& id, float amount);
//        bool MoveShapeRight(const ShapeID& id, float amount);
//        bool MoveShapeUp(const ShapeID& id, float amount);
//        bool MoveShapeDown(const ShapeID& id, float amount);
//        bool MoveShapesLeft(const std::vector<ShapeID>& ids, float amount);
//        bool MoveShapesRight(const std::vector<ShapeID>& ids, float amount);
//        bool MoveShapesUp(const std::vector<ShapeID>& ids, float amount);
//        bool MoveShapesDown(const std::vector<ShapeID>& ids, float amount);
//        bool MoveShapes(const std::vector<ShapeID>& ids, ImVec2 amount);
//
//        void SaveState();
//        void UndoAction();

        //void SetPreviewImage(const sf::Image& image);
        //void GeneratePreview();

//        std::optional<std::reference_wrapper<GenericShape>> FindShape(const ShapeID& shape);
//        bool ShapeExists(const ShapeID& id) const;
//        std::pair<ImVec2, ImVec2> GetBoundingBox() const;
//
//        float MapFloat(float x, float in_min, float in_max, float out_min, float out_max);
//        void RenderLayerToBitmap(sf::RenderTexture texture);

        bool containsUnsavedChanges() const;

    private:
        LayerID m_id;
        b::string m_name;
        //sf::RenderTexture previewImage;
        bool m_containsUnsavedChanges = false;

        std::vector<std::vector<int>> m_shapesHistory;     // 0 = current state, 1 = last state, 2 = second last state, etc.
    };

    TskDocument() = default;

	void addLayer(b::string layerName = {});
    void removeLayer(size_t index);
    void removeLayer(const LayerID& id);
    [[nodiscard]] bool layerExists(const b::string& name) const;

    [[nodiscard]] const Layer& getLayer(const LayerID& id) const;
    [[nodiscard]] size_t getLayerIndex(const LayerID& id) const;
    [[nodiscard]] const std::vector<TskDocument::Layer>& getLayers() const;

    [[nodiscard]] const Layer& getActiveLayer() const { return m_layers[m_activeLayerIndex]; }
    [[nodiscard]] Layer& getActiveLayer() { return m_layers[m_activeLayerIndex]; }
    [[nodiscard]] size_t getActiveLayerIndex() const { return m_activeLayerIndex; }

    [[nodiscard]] sf::Color getCanvasColor() const { return m_canvasColor; }
    void setCanvasColor(const sf::Color& color) { m_canvasColor = color; }

//	void GeneratePreviews() {
//		content.GeneratePreviews();
//	}

//	const DocumentLayer& GetActiveLayer() {
//		return content.GetActiveLayer();
//	}

//    DocumentLayer DuplicateActiveLayer() {
//		return content.GetActiveLayer().Duplicate();
//	}

	void moveLayerFront(const LayerID& id) {
        auto toMoveIndex = getLayerIndex(id);
        auto otherIndex = toMoveIndex + 1;

        if (otherIndex < m_layers.size()) {
            std::swap(m_layers[toMoveIndex], m_layers[otherIndex]);
        }
	}

	void moveLayerBack(const LayerID& id) {
        auto toMoveIndex = getLayerIndex(id);
        auto otherIndex = static_cast<int>(toMoveIndex) - 1;

        if (otherIndex >= 0) {
            std::swap(m_layers[toMoveIndex], m_layers[otherIndex]);
        }
	}

//	void ShowPropertiesWindow(ShapeID id) {
//		auto opt = content.GetActiveLayer().FindShape(id);
//
//		if (opt.has_value()) {
//			if (opt.value().get().ShowPropertiesWindow()) {
//                m_fileChanged = true;
//			}
//		}
//	}

	void UpdateWindowTitle();

	bool SaveFile(bool saveAs = false);
	bool OpenFile();
	bool OpenEmptyFile();
	bool OpenFile(const b::fs::path& path, bool silent = false);

	sf::Image ExportImage(bool transparent = true, float dpi = 300);

    [[nodiscard]] bool containsUnsavedChanges() const;
    [[nodiscard]] b::string getVisualFilename() const { return m_visualFilename.string(); }

    void centerOnCanvas(b::Canvas& canvas);
    void renderToCanvas(b::Canvas& canvas);
    void renderGrid(b::Canvas& canvas);
    void renderLayers(b::Canvas& canvas);

    TskDocument(const TskDocument& other) = delete;
    TskDocument& operator=(const TskDocument& other) = delete;
    TskDocument(TskDocument&& other) noexcept = default;
    TskDocument& operator=(TskDocument&& other) noexcept = default;

private:
    std::vector<Layer> m_layers;    // 0 = backmost layer, ..., size()-1 = frontmost layer
    size_t m_activeLayerIndex = 0;

    sf::Color m_canvasColor = TskSettings::Get(TskSetting::DEFAULT_NEW_CANVAS_COLOR);

    bool m_containsUnsavedChanges = false;
    b::fs::path m_visualFilename = TskSettings::Get(TskSetting::DEFAULT_TSK_NEW_FILE_FILENAME);
    b::fs::path m_absoluteFilepath;

    bool m_firstRenderPass = true;
    sf::View m_cameraView;
    double m_gridDecadeFactor = 1.0;
    b::BatchRenderer m_batchRenderer;
};
