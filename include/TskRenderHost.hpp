#pragma once

#include "TskDocument.hpp"

class TskRenderHost {
public:
    TskRenderHost();

    void resetView(b::Canvas& canvas);
    void renderShapes(b::Canvas& canvas);
    void render(b::Canvas& canvas, const TskDocument& document);

private:
    void renderGrid(b::Canvas& canvas);

    double m_gridSpacingFactor = 1.0;

    sf::View m_view;
    bool m_firstRenderPass = true;
    b::BatchRenderer m_batchRenderer;
};