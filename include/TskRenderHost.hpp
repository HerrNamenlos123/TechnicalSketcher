#pragma once

#include "TskDocument.hpp"

class TskRenderHost {
public:
    TskRenderHost();

    void resetView();

    void onMouseScroll(const b::Events::MouseWheelScrollEvent& event);
    void onMouseMove(const b::Events::MouseMoveEvent& event);

    void renderShapes();
    void render(const TskDocument& document);

private:
    void renderGrid();

    double m_gridSpacingFactor = 1.0;

    b::Window* m_window = nullptr;
    sf::View m_view;
    bool m_firstRenderPass = true;
    b::BatchRenderer m_batchRenderer;
};