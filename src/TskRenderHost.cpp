
#include "pch.hpp"
#include "Tsk.hpp"
#include "TskRenderHost.hpp"

TskRenderHost::TskRenderHost() {

}

void TskRenderHost::resetView(b::Canvas& canvas) {
    b::Vec2 diff = canvas.mapPixelToCoords({0, 0 }, m_view) - canvas.mapPixelToCoords(b::Vec2(canvas.getSize()), m_view);
    m_view.setSize(b::Vec2(canvas.getSize()));
    m_view.move(diff / 2.0);
    m_view.zoom(0.1f);
}

void TskRenderHost::renderShapes(b::Canvas& canvas) {
    m_batchRenderer.drawRect({ -5, -5 }, { 5, 5 }, sf::Color::Red, 0.1f);
    canvas.draw(m_batchRenderer);
    m_batchRenderer.clear();
}

void TskRenderHost::render(b::Canvas& canvas, const TskDocument& document) {

    if (canvas.mouse.leftButtonPressed) {
        auto deltaUnits = canvas.mapPixelToCoords({ 0, 0 }, m_view) -
                          canvas.mapPixelToCoords(b::Vec2(canvas.mouse.posDelta.x, -canvas.mouse.posDelta.y), m_view);
        m_view.move(deltaUnits);
    }

    if (canvas.mouse.scrollDelta.length() != 0) {
        auto mouseToCenter = b::Vec2(m_view.getCenter()) - canvas.mapPixelToCoords(canvas.mouse.pos, m_view);
        double zoomFactor = 1.0 - (canvas.mouse.scrollDelta.x + canvas.mouse.scrollDelta.y) * 0.1;
        m_view.zoom((float) zoomFactor);
        m_view.move((mouseToCenter * zoomFactor - mouseToCenter) * b::Vec2(1, -1));
    }

    if (m_firstRenderPass) {        // Do this here because the window size might not be known earlier
        resetView(canvas);
        m_firstRenderPass = false;
    }

    canvas.clear(document.getCanvasColor());
    renderGrid(canvas);

    canvas.setView(m_view);
    renderShapes(canvas);
}

void TskRenderHost::renderGrid(b::Canvas& canvas) {
    // Disable the view while rendering the grid
    canvas.setView(sf::View(b::Vec2(canvas.getSize()) / 2.0, b::Vec2(canvas.getSize())));

    b::Color gridLineColor = TskSettings::Get(TskSetting::DOCUMENT_GRID_LINE_COLOR);
    double gridLineWidth = TskSettings::Get(TskSetting::DOCUMENT_GRID_LINE_WIDTH);
    auto leftUpperMostPixelCoords = canvas.mapPixelToCoords({0, 0 }, m_view);

    double unit = std::floor(leftUpperMostPixelCoords.x / m_gridSpacingFactor) * m_gridSpacingFactor;
    double x = 0;
    while (x < canvas.getSize().x) {
        x = canvas.mapCoordsToPixel({static_cast<float>(unit), 0 }, m_view).x;
        m_batchRenderer.drawLine({ x, 0 }, { x, b::Vec2(canvas.getSize()).y }, gridLineColor, gridLineWidth);
        unit += m_gridSpacingFactor;
    }

    unit = std::floor(leftUpperMostPixelCoords.y / m_gridSpacingFactor) * m_gridSpacingFactor;
    double y = 0;
    while (y < canvas.getSize().y) {
        y = canvas.mapCoordsToPixel({0, static_cast<float>(unit) }, m_view).y;
        m_batchRenderer.drawLine({ 0, y }, {b::Vec2(canvas.getSize()).x, y }, gridLineColor, gridLineWidth);
        unit += m_gridSpacingFactor;
    }

    canvas.draw(m_batchRenderer);
    m_batchRenderer.clear();
}