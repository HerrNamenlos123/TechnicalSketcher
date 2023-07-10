#pragma once

#include "pch.hpp"
#include "TskHost.hpp"

struct TskWindowContext : public b::PyContext {
    bool dummy = false;

    B_DEF_PY_CONTEXT_FUNC(
        B_DEF_PY_CONTEXT_SUBCLASS(TskWindowContext, dummy);
    )
};

class TskWindow : public b::PyWindow<TskWindowContext, "TskWindowContext"> {
public:
    TskWindow() = default;

    sf::Image m_tskIcon;
    TskHost m_tskHost;

    void onAttach() override;
    void onUpdate() override;
    void onRender() override;
    void onDetach() override;
};