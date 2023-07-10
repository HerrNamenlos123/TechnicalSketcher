#pragma once

#include "pch.hpp"
#include "TskWindow.hpp"
#include "TskSplashWindow.hpp"

enum class TskWindowMode {
    Splash,
    Main
};

struct TskContext : public b::PyContext {
    TskWindowMode windowMode = TskWindowMode::Splash;
//    sf::Time lastScreenRefresh = sf::seconds(0);
//    sf::Time passiveScreenTime = sf::seconds(5.0);

    B_DEF_PY_CONTEXT_FUNC(
        B_DEF_PY_CONTEXT_SUBCLASS(TskContext, windowMode);
        b::py::enum_<TskWindowMode>(module, "TskWindowMode")
            .value("Splash", TskWindowMode::Splash)
            .value("Main", TskWindowMode::Main)
            .export_values();
    )
};

class Tsk : public b::PyApplication<TskContext, "TskContext"> {
public:
    Tsk() = default;

    TskWindow mainWindow;
    TskSplashWindow splashWindow;

    void onSetup() override;
    void onUpdate() override;
    void onRender() override;
    void onExit() override;

    B_DEF_PY_APPLICATION(Tsk);
};
