
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
        default:
            return {};
    }
}