#pragma once

// The version tag is loaded from the file "version.txt"

#define APPLICATION_NAME "TechnicalSketcher"

#define UPDATER_LOG_FILENAME "update.log.txt"
#define UPDATER_LOGGER_NAME "TechnicalSketcher-Updater"
#define UPDATE_PACKAGE_URL "https://api.github.com/repos/HerrNamenlos123/TechnicalSketcher/releases/latest"
#define UPDATE_FILENAME "AutomaticUpdaterRelease.zip"

#define DEFAULT_FILENAME "Unnamed.tsk"
#define CLIPBOARD_FORMAT "com.TechnicalSketcher.Shapes"

#define RECENT_FILES_FILENAME "recent.json"
#define SETTINGS_FILENAME "settings.json"

#define JSON_FILE_TYPE "technicalsketcher"
#define JSON_FILE_VERSION 1
#define JSON_SETTINGS_TYPE "technicalsketcher"
#define JSON_SETTINGS_VERSION 1

#define IMGUI_FILENAME "imgui.ini"

#define DEFAULT_WINDOW_WIDTH 1400
#define DEFAULT_WINDOW_HEIGHT 800
#define SCREEN_SIZE_MARGIN 1.1f
#define GUI_PREVIEWWINDOW_SIZE 100

#define DEFAULT_BACKGROUND_COLOR ImVec4(255, 255, 255, 255)
#define COLOR_TRANSPARENT ImVec4(255, 255, 255, 0)
#define EXPORT_BACKGROUND_COLOR ImVec4(255, 255, 255, 255)
#define EXPORT_FALLOFF 1

#define DEFAULT_LINE_THICKNESS 1
#define DEFAULT_LINE_COLOR ImVec4(0, 0, 0, 255)

namespace TskConfig {

} // namespace TskConfig
