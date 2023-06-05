
#include "pch.hpp"
#include "TskRenderHost.hpp"

void TskRenderHost::render(const TskDocument& document, b::BaseWindow* window) {
    window->clear(document.getCanvasColor());
}

void TskRenderHost::renderGrid() {

}