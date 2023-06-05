#pragma once

#include "pch.hpp"

struct TskWindowContext : public b::PyContext {
    bool test = false;

    B_DEF_PY_CONTEXT(TskWindowContext, test);
};

class TskWindow : public b::PyWindow<TskWindowContext, "TskWindowContext"> {
public:
    TskWindow() = default;

    void initSplashScreen();
    void switchToMainScreen();
    bool m_splashScreen = true;

    void attach() override;
    void update() override;
    void detach() override;
};