
#include "TskSplashWindow.hpp"
#include "resources/splashscreen_png.hpp"

void TskSplashWindow::onAttach() {
    sf::Texture splash;
    auto splashResource = Resources::SPLASHSCREEN_PNG;
    if (splash.loadFromMemory(splashResource.data(), splashResource.size())) {
        this->create(splash.getSize(), "TechnicalSketcher", sf::Style::None);
    }
}

void TskSplashWindow::onUpdate() {

}

void TskSplashWindow::onDetach() {

}