#pragma once

#define APPLICATION_NAME "TechnicalSketcher"
#define DEFAULT_FILENAME "Unnamed.tsk"
#define CLIPBOARD_FORMAT "com.TechnicalSketcher.Shapes"

#define RECENT_FILES_FILENAME "recent.json"
#define SETTINGS_FILENAME "settings.json"

#define JSON_FILE_TYPE "technicalsketcher"
#define JSON_FILE_VERSION 1
#define JSON_SETTINGS_TYPE "technicalsketcher"
#define JSON_SETTINGS_VERSION 1

#define IMGUI_FILENAME "imgui.ini"

#define UPDATER_EXE_NAME "updater.exe"
#define ENABLE_PROFILING true

#define DEFAULT_WINDOW_WIDTH 1400
#define DEFAULT_WINDOW_HEIGHT 800
#define SCREEN_SIZE_MARGIN 1.1f
#define GUI_PREVIEWWINDOW_SIZE 100

#define MAX_NUMBER_OF_UNDOS 20
#define MAX_NUMBER_OF_RECENT_FILES 5

#define DEFAULT_BACKGROUND_COLOR glm::vec4(255, 255, 255, 255)
#define COLOR_TRANSPARENT glm::vec4(255, 255, 255, 0)
#define EXPORT_BACKGROUND_COLOR glm::vec4(255, 255, 255, 255)
#define EXPORT_FALLOFF 1

#define DEFAULT_LINE_THICKNESS 1
#define DEFAULT_LINE_COLOR glm::vec4(0, 0, 0, 255)
