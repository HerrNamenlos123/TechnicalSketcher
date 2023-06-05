#pragma once

#include "pch.hpp"

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

    void onAttach() override;
    void onUpdate() override;
    void onDetach() override;
};