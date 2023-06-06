
#include "TskSplashWindow.hpp"
#include "resources/splashscreen_png.hpp"
#include "resources/assets/TechnicalSketcher_png.hpp"

void TskSplashWindow::onAttach() {
    m_splashResource = Resources::splashscreen_png;
    if (m_splashTexture.loadFromMemory(m_splashResource.data(), m_splashResource.size())) {
        this->create(m_splashTexture.getSize(), "TechnicalSketcher", sf::Style::None);
        m_splashSprite.setTexture(m_splashTexture);
        m_splashSprite.setPosition({ 0, 0 });
    }

    if (!m_tskIcon.loadFromMemory(Resources::assets::TechnicalSketcher_png.data(), Resources::assets::TechnicalSketcher_png.size())) {
        throw std::invalid_argument("Failed to load TechnicalSketcher.png icon from resource. This should not fail!");
    }
    this->setIcon(m_tskIcon);
}

void TskSplashWindow::onUpdate() {
    draw(m_splashSprite);
}

void TskSplashWindow::onDetach() {

}