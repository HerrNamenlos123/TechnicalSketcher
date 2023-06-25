
#include "pch.hpp"
#include "Tsk.hpp"
#include "TskRenderHost.hpp"

TskRenderHost::TskRenderHost() {
    auto& window = Tsk::get().mainWindow;

    window.attachEventHandler<b::Events::MouseWheelScrollEvent>([&](const auto& event) {
        auto mouseToCenter = m_view.getCenter() - window.mapPixelToCoords(window.getMousePos(), m_view);
        double zoomFactor = 1.0 + event.delta * 0.1;

        m_view.zoom((float)zoomFactor);
        m_view.move(mouseToCenter * zoomFactor - mouseToCenter);
    });

    window.attachEventHandler<b::Events::MouseMoveEvent>([&](const auto& event) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            auto deltaUnits = window.mapPixelToCoords({ 0, 0 }, m_view) - window.mapPixelToCoords(event.delta, m_view);
            m_view.move(deltaUnits);
        }
    });
}

void TskRenderHost::renderShapes() {
    auto& window = Tsk::get().mainWindow;
    m_batchRenderer.drawRect({ -20, -10 }, { 1, 1 }, sf::Color::Red, 0.3f);
    window.draw(m_batchRenderer);
    m_batchRenderer.clear();
}

void TskRenderHost::render(const TskDocument& document) {
    auto& window = Tsk::get().mainWindow;

    if (m_firstRenderPass) {        // Do this here because the window size might not be known earlier
        auto diff = window.mapPixelToCoords({ 0, 0 }, m_view) - window.mapPixelToCoords(window.getSize(), m_view);
        m_view.setSize(window.getSize());
        m_view.move(diff / 2.0);
        m_view.zoom(0.1f);
        m_firstRenderPass = false;
    }

    window.clear(document.getCanvasColor());

    renderGrid();

    window.setView(m_view);
    renderShapes();
}

void TskRenderHost::renderGrid() {
    auto& window = Tsk::get().mainWindow;

    // Disable the view while rendering the grid
    window.setView(sf::View(window.getSize() / 2.0, window.getSize()));

    b::Color majorGridLineColor = TskSettings::Get(TskSetting::DOCUMENT_MAJOR_GRID_LINE_COLOR);
    b::Color minorGridLineColor = TskSettings::Get(TskSetting::DOCUMENT_MINOR_GRID_LINE_COLOR);
    double majorGridLineWidth = TskSettings::Get(TskSetting::DOCUMENT_MAJOR_GRID_LINE_WIDTH);
    double minorGridLineWidth = TskSettings::Get(TskSetting::DOCUMENT_MINOR_GRID_LINE_WIDTH);
    double minGridLineSpacingPx = TskSettings::Get(TskSetting::DOCUMENT_MIN_GRID_LINE_SPACING_PX);

    // First the center lines
    m_batchRenderer.drawLine(b::Vec2(0, window.mapCoordsToPixel({0, 0 }, m_view).y),
                             b::Vec2(window.getSize().x, window.mapCoordsToPixel({0, 0 }, m_view).y),
                             sf::Color::Blue, 1.f);
    m_batchRenderer.drawLine(b::Vec2(window.mapCoordsToPixel({0, 0 }, m_view).x, 0),
                             b::Vec2(window.mapCoordsToPixel({0, 0 }, m_view).x, window.getSize().y),
                             sf::Color::Blue, 1.f);

    // Now right lines
    int unitX = 0;
    double pixelX = 0;
    do {
        unitX += 1;
        pixelX = window.mapCoordsToPixel(b::Vec2(unitX, 0), m_view).x;

        if (unitX % 10 == 0) {
            m_batchRenderer.drawLine(b::Vec2(pixelX, 0), b::Vec2(pixelX, window.getSize().y), sf::Color::Black, 1.f);
        }
        else {
            m_batchRenderer.drawLine(b::Vec2(pixelX, 0), b::Vec2(pixelX, window.getSize().y), sf::Color::Green, 0.5f);
        }
    } while (pixelX <= window.getSize().x);

    // Left lines
    unitX = 0;
    pixelX = 0;
    do {
        unitX -= 1;
        pixelX = window.mapCoordsToPixel(b::Vec2(unitX, 0), m_view).x;

        if (unitX % 10 == 0) {
            m_batchRenderer.drawLine(b::Vec2(pixelX, 0), b::Vec2(pixelX, window.getSize().y), sf::Color::Black, 1.f);
        }
        else {
            m_batchRenderer.drawLine(b::Vec2(pixelX, 0), b::Vec2(pixelX, window.getSize().y), sf::Color::Green, 0.5f);
        }
    } while (pixelX >= 0);


    // Now top lines
    int unitY = 0;
    double pixelY = 0;
    do {
        unitY += 1;
        pixelY = window.mapCoordsToPixel(b::Vec2(0, unitY), m_view).y;

        if (unitY % 10 == 0) {
            m_batchRenderer.drawLine(b::Vec2(0, pixelY), b::Vec2(window.getSize().x, pixelY), sf::Color::Black, 1.f);
        }
        else {
            m_batchRenderer.drawLine(b::Vec2(0, pixelY), b::Vec2(window.getSize().x, pixelY), sf::Color::Green, 0.5f);
        }
    } while (pixelY <= window.getSize().y);

    // Bottom lines
    unitY = 0;
    pixelY = 0;
    do {
        unitY -= 1;
        pixelY = window.mapCoordsToPixel(b::Vec2(0, unitY), m_view).y;

        if (unitY % 10 == 0) {
            m_batchRenderer.drawLine(b::Vec2(0, pixelY), b::Vec2(window.getSize().x, pixelY), sf::Color::Black, 1.f);
        }
        else {
            m_batchRenderer.drawLine(b::Vec2(0, pixelY), b::Vec2(window.getSize().x, pixelY), sf::Color::Green, 0.5f);
        }
    } while (pixelY >= 0);

    window.draw(m_batchRenderer);
    m_batchRenderer.clear();
}