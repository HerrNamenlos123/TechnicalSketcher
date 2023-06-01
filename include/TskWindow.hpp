#pragma once

#include "pch.hpp"

class TskWindow : public b::window {
public:
    TskWindow() = default;

    void initSplashScreen();
    void switchToMainScreen();
    bool m_splashScreen = true;

    void attach() override;
    void update() override;
    void detach() override;
};