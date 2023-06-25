
#include "pch.hpp"
#include "Tsk.hpp"
#include "TskRenderHost.hpp"

TskRenderHost::TskRenderHost() {
    auto& window = Tsk::get().mainWindow;

    m_view.setCenter(window.getSize() / 2.0);

    window.attachEventHandler<b::Events::MouseWheelScrollEvent>([&](const auto& event) {
        m_view.zoom(1.0f + event.delta * 0.1f);
    });
    window.attachEventHandler<b::Events::MouseMoveEvent>([&](const auto& event) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            auto d = -window.mapPixelToCoords(event.delta, m_view);
            m_view.move(d);
            b::log::debug("Mouse moved: {}/{}", d.x, d.y);
        }
    });
}

void TskRenderHost::render(const TskDocument& document) {
    auto& window = Tsk::get().mainWindow;

    window.clear(document.getCanvasColor());
    m_view.setSize(window.getSize());
    m_view.zoom(0.1f);

    renderGrid();
}

void TskRenderHost::renderGrid() {
    auto& window = Tsk::get().mainWindow;

    // Disable the view while rendering the grid
    window.setView(sf::View(window.getSize() / 2.0, window.getSize()));

    // First the center lines
    m_batchRenderer.drawLine(b::vec2(0, window.mapCoordsToPixel({ 0, 0 }, m_view).y),
                             b::vec2(window.getSize().x, window.mapCoordsToPixel({ 0, 0 }, m_view).y),
                             sf::Color::Blue, 1.f);
    m_batchRenderer.drawLine(b::vec2(window.mapCoordsToPixel({ 0, 0 }, m_view).x, 0),
                             b::vec2(window.mapCoordsToPixel({ 0, 0 }, m_view).x, window.getSize().y),
                             sf::Color::Blue, 1.f);

    window.draw(m_batchRenderer);
    m_batchRenderer.clear();
return;
    // Now right lines
    int unitX = 0;
    double pixelX = 0;
    do {
        unitX += 1;
        pixelX = window.mapCoordsToPixel(b::vec2(unitX, 0), m_view).x;

        if (unitX % 10 == 0) {
            m_batchRenderer.drawLine(b::vec2(pixelX, 0), b::vec2(pixelX, window.getSize().y), sf::Color::Black, 1.f);
        }
        else {
            m_batchRenderer.drawLine(b::vec2(pixelX, 0), b::vec2(pixelX, window.getSize().y), sf::Color::Green, 0.5f);
        }
    } while (pixelX <= window.getSize().x);

    // Left lines
    unitX = 0;
    pixelX = 0;
    do {
        unitX -= 1;
        pixelX = window.mapCoordsToPixel(b::vec2(unitX, 0), m_view).x;

        if (unitX % 10 == 0) {
            m_batchRenderer.drawLine(b::vec2(pixelX, 0), b::vec2(pixelX, window.getSize().y), sf::Color::Black, 1.f);
        }
        else {
            m_batchRenderer.drawLine(b::vec2(pixelX, 0), b::vec2(pixelX, window.getSize().y), sf::Color::Green, 0.5f);
        }
    } while (pixelX >= 0);


    // Now top lines
    int unitY = 0;
    double pixelY = 0;
    do {
        unitY += 1;
        pixelY = window.mapCoordsToPixel(b::vec2(0, unitY), m_view).y;

        if (unitY % 10 == 0) {
            m_batchRenderer.drawLine(b::vec2(0, pixelY), b::vec2(window.getSize().x, pixelY), sf::Color::Black, 1.f);
        }
        else {
            m_batchRenderer.drawLine(b::vec2(0, pixelY), b::vec2(window.getSize().x, pixelY), sf::Color::Green, 0.5f);
        }
    } while (pixelY <= window.getSize().y);

    // Bottom lines
    unitY = 0;
    pixelY = 0;
    do {
        unitY -= 1;
        pixelY = window.mapCoordsToPixel(b::vec2(0, unitY), m_view).y;

        if (unitY % 10 == 0) {
            m_batchRenderer.drawLine(b::vec2(0, pixelY), b::vec2(window.getSize().x, pixelY), sf::Color::Black, 1.f);
        }
        else {
            m_batchRenderer.drawLine(b::vec2(0, pixelY), b::vec2(window.getSize().x, pixelY), sf::Color::Green, 0.5f);
        }
    } while (pixelY >= 0);

    window.draw(m_batchRenderer);
    m_batchRenderer.clear();
}