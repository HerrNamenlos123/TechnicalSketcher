#pragma once

#include "pch.hpp"

struct TskWindowContext : public b::py_context {

    B_DEF_PY_CONTEXT(TskWindowContext);
};

class TskWindow : public b::py_window<TskWindowContext, "TskWindowContext"> {
public:
    TskWindow() = default;

    void initSplashScreen();
    void switchToMainScreen();
    bool m_splashScreen = true;

    void attach() override;
    void update() override;
    void detach() override;
};