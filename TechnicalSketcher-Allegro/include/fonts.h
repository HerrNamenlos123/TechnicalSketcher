#pragma once

#include "pch.h"

extern ImFont* font_pt9;
extern ImFont* font_pt10;
extern ImFont* font_pt11;
extern ImFont* font_pt12;
extern ImFont* font_pt13;
extern ImFont* font_pt14;
extern ImFont* font_pt15;
extern ImFont* font_pt16;
extern ImFont* font_pt18;
extern ImFont* font_pt20;
extern ImFont* font_pt22;
extern ImFont* font_pt25;
extern ImFont* font_pt28;
extern ImFont* font_pt30;
extern ImFont* font_pt35;
extern ImFont* font_pt40;

/// <summary>
/// Loads ImGui fonts into memory, default font is Calibri.
/// Font size goes from 9 pt up to 40 pt.
/// Should be called after ImGui initialization in Application::setup()
/// </summary>
void loadImGuiFonts();
