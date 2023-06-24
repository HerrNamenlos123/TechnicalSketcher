
#include "pch.hpp"
#include "Tsk.hpp"
#include "TskRenderHost.hpp"

void TskRenderHost::render(const TskDocument& document) {
    auto* window = Tsk::get().mainWindow;

    window->clear(document.getCanvasColor());
    renderGrid();
}

void TskRenderHost::renderGrid() {
    auto* window = Tsk::get().mainWindow;

    b::vec2 centerOffset = window->getMousePos();
    float scale = 1.f;

    // First the center lines
    window->batchRenderer.drawLine(centerOffset + b::vec2(window->getSize().x / 2.f, 0),
                                   { window->getSize().x / 2.f, window->getSize().y * 1.f },
                                   sf::Color::Black, 1.f);
    window->batchRenderer.drawLine({ 0, window->getSize().y / 2.f },
                                   { window->getSize().x * 1.f, window->getSize().y / 2.f },
                                   sf::Color::Black, 1.f);
}