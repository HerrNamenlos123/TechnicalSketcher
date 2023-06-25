#pragma once

#include "TskDocument.hpp"

class TskRenderHost {
public:
    TskRenderHost();

    void renderShapes();
    void render(const TskDocument& document);

private:
    void renderGrid();

    sf::View m_view;
    bool m_firstRenderPass = true;
    b::BatchRenderer m_batchRenderer;
};