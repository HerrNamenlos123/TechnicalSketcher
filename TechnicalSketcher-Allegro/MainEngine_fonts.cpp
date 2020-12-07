
#include "pch.h"
#include "MainEngine.h"

void MainEngine::loadFonts() {

	ImGuiIO& io = ImGui::GetIO();
	font_pt9  = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 9);
	font_pt10 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 10);
	font_pt11 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 11);
	font_pt12 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 12);
	font_pt13 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 13);
	font_pt14 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 14);
	font_pt15 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 15);
	font_pt16 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 16);
	font_pt18 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 18);
	font_pt20 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 20);
	font_pt22 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 22);
	font_pt25 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 25);
	font_pt28 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 28);
	font_pt30 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 30);
	font_pt35 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 35);
	font_pt40 = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 40);
}
