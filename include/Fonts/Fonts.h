#pragma once

#include "pch.h"
#include "FontLoader.h"

struct FontContainer : public Battery::FontContainer {
	const ImWchar icons_ranges[3] = { 0xe005, 0xf8ff, 0 };
	ImFont* sansFont9 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_OPENSANS, 9);
	ImFont* sansFont14 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_OPENSANS, 14);
	ImFont* sansFont17 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_OPENSANS, 17);
	ImFont* sansFont22 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_OPENSANS, 22);
	ImFont* materialFont35 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_MATERIAL_ICONS, 35, nullptr, icons_ranges);
	ImFont* materialFont22 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_MATERIAL_ICONS, 22, nullptr, icons_ranges);
	ImFont* segoeFont22 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(FONT_SEGOE_MDL2, 22, nullptr, icons_ranges);
};
