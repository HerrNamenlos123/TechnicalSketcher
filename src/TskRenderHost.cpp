
#include "pch.hpp"
#include "Tsk.hpp"
#include "TskRenderHost.hpp"

TskRenderHost::TskRenderHost() {

    // Zooming
    Tsk::get().mainWindow.attachEventHandler<b::Events::MouseWheelScrollEvent>([this](const auto& event) {
        onMouseScroll(event);
    });

    // Panning
    Tsk::get().mainWindow.attachEventHandler<b::Events::MouseMoveEvent>([this](const auto& event) {
        onMouseMove(event);
    });
}

void TskRenderHost::resetView() {
    auto diff = m_window->mapPixelToCoords({ 0, 0 }, m_view) - m_window->mapPixelToCoords(m_window->getSize(), m_view);
    m_view.setSize(m_window->getSize());
    m_view.move(diff / 2.0);
    m_view.zoom(0.1f);
}

void TskRenderHost::renderShapes() {
    m_batchRenderer.drawRect({ -5, -5 }, { 5, 5 }, sf::Color::Red, 0.1f);
    m_window->draw(m_batchRenderer);
    m_batchRenderer.clear();
}

void TskRenderHost::render(const TskDocument& document) {
    m_window = &Tsk::get().mainWindow;

    if (m_firstRenderPass) {        // Do this here because the window size might not be known earlier
        resetView();
        m_firstRenderPass = false;
    }

    m_window->clear(document.getCanvasColor());

    renderGrid();

    m_window->setView(m_view);
    renderShapes();
}

void TskRenderHost::renderGrid() {
    // Disable the view while rendering the grid
    m_window->setView(sf::View(m_window->getSize() / 2.0, m_window->getSize()));

    b::Color gridLineColor = TskSettings::Get(TskSetting::DOCUMENT_GRID_LINE_COLOR);
    double gridLineWidth = TskSettings::Get(TskSetting::DOCUMENT_GRID_LINE_WIDTH);
    auto leftUpperMostPixelCoords = m_window->mapPixelToCoords({ 0, 0 }, m_view);

    double unit = std::floor(leftUpperMostPixelCoords.x / m_gridSpacingFactor) * m_gridSpacingFactor;
    double x = 0;
    while (x < m_window->getSize().x) {
        x = m_window->mapCoordsToPixel({ unit, 0 }, m_view).x;
        m_batchRenderer.drawLine({ x, 0 }, { x, m_window->getSize().y }, gridLineColor, gridLineWidth);
        unit += m_gridSpacingFactor;
    }

    unit = std::floor(leftUpperMostPixelCoords.y / m_gridSpacingFactor) * m_gridSpacingFactor;
    double y = 0;
    while (y < m_window->getSize().y) {
        y = m_window->mapCoordsToPixel({ 0, unit }, m_view).y;
        m_batchRenderer.drawLine({ 0, y }, { m_window->getSize().x, y }, gridLineColor, gridLineWidth);
        unit += m_gridSpacingFactor;
    }

    m_window->draw(m_batchRenderer);
    m_batchRenderer.clear();
}