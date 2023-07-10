#pragma once

#include "pch.hpp"

struct TskSplashWindowContext : public b::PyContext {
    bool dummy = false;

    B_DEF_PY_CONTEXT_FUNC(
        B_DEF_PY_CONTEXT_SUBCLASS(TskSplashWindowContext, dummy);
    )
};

class TskSplashWindow : public b::PyWindow<TskSplashWindowContext, "TskSplashWindowContext"> {
public:
    TskSplashWindow() = default;

    sf::Image m_tskIcon;

    b::Resource m_splashResource;
    sf::Texture m_splashTexture;
    sf::Sprite m_splashSprite;

    void onAttach() override;
    void onUpdate() override;
    void onRender() override;
    void onDetach() override;
};