#pragma once

#include "pch.hpp"
#include "TskWindow.hpp"

class Tsk : public b::application {
public:

    sf::Time lastScreenRefresh = sf::seconds(0);
    sf::Time passiveScreenTime = sf::seconds(5.0);

    inline static std::shared_ptr<TskWindow> s_mainWindow;

    Tsk() = default;

    void setup() override {
        this->attachWindow<TskWindow>(s_mainWindow);
    }

    void update() override {

    }

    void cleanup() override {
        s_mainWindow.reset();
    }

    void OnEvent(sf::Event e, bool& handled);

    inline static Tsk& get() { return dynamic_cast<Tsk&>(b::application::get()); }
};
