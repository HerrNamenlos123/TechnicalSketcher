#pragma once

// Values and flags which are used when program is freshly started

#define DEFAULT_WINDOW_WIDTH  1300
#define DEFAULT_WINDOW_HEIGHT 700
#define DEFAULT_WINDOW_WIDTH  1800
#define DEFAULT_WINDOW_HEIGHT 1000
#define DEFAULT_FILENAME "Unnamed.tsk"

// #define DEFAULT_WINDOW_FLAGS (WINDOW_RESIZABLE | WINDOW_MAXIMIZED)
#define DEFAULT_WINDOW_FLAGS (WINDOW_RESIZABLE)


// Used in loading and saving files
#define MAX_PATH_LENGTH 1024


// Some aesthetic values influencing the visual layout

#define GUI_RIBBON_HEIGHT 100
#define GUI_LEFT_BAR_WIDTH 200
#define GUI_LAYER_WINDOW_HEIGHT 300
#define GUI_TOOLBOX_WINDOW_HEIGHT 200
#define GUI_MOUSEINFO_WINDOW_HEIGHT 30

#define GUI_PREVIEWWINDOW_SIZE 100


enum CursorTool {
	TOOL_SELECT,
	TOOL_LINE,
	TOOL_LINE_STRIP
};
