#pragma once

class ApplicationRenderer {

    std::unique_ptr<Battery::Scene> scene;

    ApplicationRenderer();

public:
    ~ApplicationRenderer();

    // Visuals
    float gridLineColor = 0;	// Grayscale
    float gridLineWidth = 1;
    float gridAlphaFactor = 1;
    float gridAlphaOffset = 0;
    float gridAlphaMax = 50;
    //float gridLineFalloff = 0.7;
    glm::vec4 disabledLineColor = glm::vec4(200, 200, 200, 255);
    glm::vec4 normalLineColor = glm::vec4(0, 0, 0, 255);
    glm::vec4 hoveredLineColor = glm::vec4(252, 132, 3, 255);
    glm::vec4 selectedLineColor = glm::vec4(255, 0, 0, 255);
    glm::vec4 selectionBoxOutlineColor = glm::vec4(252, 132, 3, 255);
    float selectionBoxOutlineThickness = 1.f;
    glm::vec4 selectionBoxFillColor = glm::vec4(200, 20, 0, 15);

    float previewPointSize = 4;

    static void Load();
    static void Unload();

    static void BeginFrame(bool background = true);
    static void EndFrame();

    static void DrawLineWorkspace(const glm::vec2& point1, const glm::vec2& point2, float thickness, 
        const glm::vec4& color, float falloff = BATTERY_ANTIALIASING_LINE_FALLOFF);
    static void DrawLineScreenspace(const glm::vec2& point1, const glm::vec2& point2, float thickness, 
        const glm::vec4& color, float falloff = BATTERY_ANTIALIASING_LINE_FALLOFF);

    static void DrawRectangleWorkspace(const glm::vec2& point1, const glm::vec2& point2, float outlineThickness,
        const glm::vec4& outlineColor, const glm::vec4& fillColor, float falloff = BATTERY_ANTIALIASING_LINE_FALLOFF);
    static void DrawRectangleScreenspace(const glm::vec2& point1, const glm::vec2& point2, float outlineThickness,
        const glm::vec4& outlineColor, const glm::vec4& fillColor, float falloff = BATTERY_ANTIALIASING_LINE_FALLOFF);



   static void DrawSelectionBoxInfillRectangle(const glm::vec2& point1, const glm::vec2& point2);
   static void DrawSelectionBoxOutlineRectangle(const glm::vec2& point1, const glm::vec2& point2);

   static void DrawPreviewPoint(const glm::vec2& position);
   static void DrawGrid();

    static ApplicationRenderer& GetInstance() {
        static ApplicationRenderer instance;
        return instance;
    }

private:
    ApplicationRenderer(ApplicationRenderer const&) = delete;
    void operator=(ApplicationRenderer const&) = delete;

};
