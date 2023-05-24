#pragma once

class ApplicationRenderer {

    ApplicationRenderer() {}

public:
    ~ApplicationRenderer() {}

    // Visuals
    float gridLineColor = 0;	// Grayscale
    float gridLineWidth = 1;
    float gridAlphaFactor = 10;
    float gridAlphaOffset = 0;
    float gridAlphaMax = 50;
    //float gridLineFalloff = 0.7;
    ImVec4 disabledLineColor = ImVec4(200, 200, 200, 255);
    ImVec4 normalLineColor = ImVec4(0, 0, 0, 255);
    ImVec4 hoveredLineColor = ImVec4(252, 132, 3, 255);
    ImVec4 selectedLineColor = ImVec4(255, 0, 0, 255);
    ImVec4 selectionBoxOutlineColor = ImVec4(252, 132, 3, 255);
    float selectionBoxOutlineThickness = 1.f;
    ImVec4 selectionBoxFillColor = ImVec4(200, 20, 0, 15);

    float previewPointSize = 4;

    static void DrawLineWorkspace(const ImVec2& point1, const ImVec2& point2, float thickness, 
        const ImColor& color);
    static void DrawLineScreenspace(const ImVec2& point1, const ImVec2& point2, float thickness, 
        const ImColor& color);
    static void DrawLineExport(const ImVec2& point1, const ImVec2& point2, float thickness, 
        const ImVec4& color, ImVec2 min, ImVec2 max, float width, float height);

    static void DrawCircleWorkspace(const ImVec2& center, float radius, float thickness, 
        const ImColor& color);
    static void DrawCircleScreenspace(const ImVec2& center, float radius, float thickness,
        const ImColor& color);
    static void DrawCircleExport(const ImVec2& center, float radius, float thickness,
        const ImVec4& color, ImVec2 min, ImVec2 max, float width, float height);

    static void DrawArcWorkspace(const ImVec2& center, float radius, float startAngle, float endAngle, float thickness,
        const ImColor& color);
    static void DrawArcScreenspace(const ImVec2& center, float radius, float startAngle, float endAngle, float thickness,
        const ImColor& color);
    static void DrawArcExport(const ImVec2& center, float radius, float startAngle, float endAngle, float thickness,
        const ImVec4& color, ImVec2 min, ImVec2 max, float width, float height);

    static void DrawRectangleWorkspace(const ImVec2& point1, const ImVec2& point2, float outlineThickness,
        const ImColor& outlineColor, const ImColor& fillColor);
    static void DrawRectangleScreenspace(const ImVec2& point1, const ImVec2& point2, float outlineThickness,
        const ImColor& outlineColor, const ImColor& fillColor);



   static void DrawSelectionBoxInfillRectangle(const ImVec2& point1, const ImVec2& point2);
   static void DrawSelectionBoxOutlineRectangle(const ImVec2& point1, const ImVec2& point2);

   static void DrawPreviewPoint(const ImVec2& position);
   static void DrawGrid(bool infinite = false);

    static ApplicationRenderer& GetInstance() {
        static ApplicationRenderer instance;
        return instance;
    }

    ApplicationRenderer(ApplicationRenderer const&) = delete;
    void operator=(ApplicationRenderer const&) = delete;

};
