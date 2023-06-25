#pragma once

#include "TskDocument.hpp"

class TskRenderHost {
public:
    TskRenderHost();

    void render(const TskDocument& document);

private:
    void renderGrid();

    sf::View m_view;
    b::BatchRenderer m_batchRenderer;
};