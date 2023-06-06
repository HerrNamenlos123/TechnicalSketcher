
#include "pch.hpp"
#include "Tsk.hpp"
#include "TskRenderHost.hpp"

void TskRenderHost::render(const TskDocument& document) {
    auto* window = Tsk::get().s_mainWindow;

    window->clear(document.getCanvasColor());
    renderGrid();
}

void TskRenderHost::renderGrid() {
    auto* window = Tsk::get().s_mainWindow;

    ImVec2 offset = { 0, 0 };

    // First the center line
    window->drawLine({ 100, 100 }, window->getMousePos(), sf::Color::Red, 5, b::LineCap::Round);
}