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

    void onAttach() override;
    void onUpdate() override;
    void onDetach() override;
};