#pragma once

#include "pch.hpp"
#include "TskWindow.hpp"

struct TskContext : public b::py_context {
    bool test2 = false;
    sf::Time lastScreenRefresh = sf::seconds(0);
    sf::Time passiveScreenTime = sf::seconds(5.0);

    B_DEF_PY_CONTEXT(TskContext, test2);
};

class Tsk : public b::py_application<TskContext, "TskContext"> {
public:
    Tsk() = default;

    TskWindow* s_mainWindow;

    void setup() override {
        this->attachWindow<TskWindow>(s_mainWindow);
    }

    void update() override {}
    void cleanup() override {}

    inline static Tsk& get() { return dynamic_cast<Tsk&>(b::basic_application::get()); }
};
