#pragma once

#include "pch.hpp"

struct TskWindowContext : public b::context {
    bool test1 = false;

    void define_python_types(b::py::module& module) override {
        b::py::class_<TskWindowContext>(module, "TskWindowContext")
            .def_readwrite("test1", &TskWindowContext::test1);
    }
};

class TskWindow : public b::window<TskWindowContext, "TskWindowContext"> {
public:
    TskWindow() = default;

    void initSplashScreen();
    void switchToMainScreen();
    bool m_splashScreen = true;

    void attach() override;
    void update() override;
    void detach() override;
};