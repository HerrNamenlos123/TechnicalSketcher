#pragma once

#include "pch.hpp"
#include "TskWindow.hpp"

struct TskContext : public b::context {
    bool test2 = false;
    sf::Time lastScreenRefresh = sf::seconds(0);
    sf::Time passiveScreenTime = sf::seconds(5.0);

    void define_python_types(b::py::module& module) override {
        b::py::class_<TskContext>(module, "Context")
            .def_readwrite("test2", &TskContext::test2);
    }
};

class Tsk : public b::application<TskContext, "TskContext"> {
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
