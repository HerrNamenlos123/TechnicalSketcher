#pragma once

#include "battery/core/all.hpp"
#include "battery/graphics/application.hpp"
#include "battery/filedialog/filedialog.hpp"

class MainWindow : public b::window {
public:
    MainWindow() = default;
    template<typename... Args>
    MainWindow(Args&&... args) : b::window(std::forward<Args>(args)...) {}

    void setup() override;
    void update() override;
    void cleanup() override;
};

class App : public b::application {
public:

    sf::Time lastScreenRefresh = sf::seconds(0);
    sf::Time passiveScreenTime = sf::seconds(5.0);

    inline static std::shared_ptr<MainWindow> s_mainWindow;

    App() = default;

    void setup() override {
        s_mainWindow = std::make_shared<MainWindow>(sf::Vector2u({ 1280, 720 }));
        this->add_window(s_mainWindow);
    }

    void update() override {

    }

    void cleanup() override {
        s_mainWindow.reset();
    }

    void OnEvent(sf::Event e, bool& handled);

    inline static App& get() { return dynamic_cast<App&>(b::application::get()); }
};
