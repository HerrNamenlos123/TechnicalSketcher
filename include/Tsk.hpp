#pragma once

#include "pch.hpp"
#include "TskWindow.hpp"

struct TskContext : public b::PyContext {
    bool test2 = false;
    sf::Time lastScreenRefresh = sf::seconds(0);
    sf::Time passiveScreenTime = sf::seconds(5.0);

    B_DEF_PY_CONTEXT(TskContext, test2);
};

class Tsk : public b::PyApplication<TskContext, "TskContext"> {
public:
    Tsk() = default;

    TskWindow* s_mainWindow = nullptr;

    void setup() override {
        this->attachWindow(&s_mainWindow);
    }

    void update() override {}
    void cleanup() override {}

    B_DEF_PY_APPLICATION(Tsk);
};
