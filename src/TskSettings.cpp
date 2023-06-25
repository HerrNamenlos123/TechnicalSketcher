
#include "TskSettings.hpp"

nlohmann::json TskSettings::GetStandardSetting(TskSetting setting) {
    switch (setting) {

        case TskSetting::MAX_NUMBER_OF_RECENT_FILES:
            return 10;

        case TskSetting::MAX_NUMBER_OF_UNDOS:
            return 100;

        case TskSetting::DEFAULT_TSK_NEW_FILE_FILENAME:
            return "Untitled";

        case TskSetting::DEFAULT_NEW_CANVAS_COLOR:
            return { 227, 217, 186, 255 };

        case TskSetting::DOCUMENT_MAJOR_GRID_LINE_COLOR:
            return { 255, 0, 0, 255 };

        case TskSetting::DOCUMENT_MAJOR_GRID_LINE_WIDTH:
            return 1;

        case TskSetting::DOCUMENT_MINOR_GRID_LINE_COLOR:
            return { 0, 0, 255, 255 };

        case TskSetting::DOCUMENT_MINOR_GRID_LINE_WIDTH:
            return 1;

        case TskSetting::DOCUMENT_MIN_GRID_LINE_SPACING_PX:
            return 10;

        default:
            return {};
    }
}