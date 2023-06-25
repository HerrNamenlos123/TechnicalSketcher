#pragma once

#include "battery/core.hpp"
#include "magic_enum.hpp"

#include <utility>

enum class TskSetting {
    NONE,
    MAX_NUMBER_OF_RECENT_FILES,
    MAX_NUMBER_OF_UNDOS,
    DEFAULT_TSK_NEW_FILE_FILENAME,
    DEFAULT_NEW_CANVAS_COLOR,
    DOCUMENT_MAJOR_GRID_LINE_COLOR,
    DOCUMENT_MAJOR_GRID_LINE_WIDTH,
    DOCUMENT_MINOR_GRID_LINE_COLOR,
    DOCUMENT_MINOR_GRID_LINE_WIDTH,
    DOCUMENT_MIN_GRID_LINE_SPACING_PX
};

class TskSettings {
public:
    static b::fs::path SettingsDirectory() {
        return b::Folders::AppConfigDir();
    }

    static b::fs::path GetMostRecentFile() {
        auto files = GetRecentFiles();
        if (files.empty()) {
            return {};
        }
        return files.back();
    }

    static std::vector<b::fs::path> GetRecentFiles() {
        try {
            std::vector<std::string> files = get().cache["recent_files"];
            std::vector<b::fs::path> paths;
            paths.reserve(files.size());
            for (const auto &file: files) {
                paths.emplace_back(file);
            }
            return paths;
        }
        catch (...) {
            return {};
        }
    }

    static void AddRecentFile(const b::fs::path& path) {
        auto files = GetRecentFiles();

        // If the file already exists, delete it
        if (std::find(files.begin(), files.end(), path) != files.end()) {
            files.erase(std::remove(files.begin(), files.end(), path), files.end());
        }

        // Delete first one, if there's too many
        while (files.size() > Get(TskSetting::MAX_NUMBER_OF_RECENT_FILES)) {
            files.erase(files.begin());
        }

        files.emplace_back(path);
        get().cache["recent_files"] = files;
    }

    static nlohmann::json Get(TskSetting setting) {
        if (get().cache.contains(magic_enum::enum_name(setting))) {
            return get().cache[magic_enum::enum_name(setting)];
        }
        else {
            return GetStandardSetting(setting);
        }
    }

    static void Set(TskSetting setting, nlohmann::json value) {
        get().cache[magic_enum::enum_name(setting)] = std::move(value);
    }

    static nlohmann::json GetStandardSetting(TskSetting setting);

    TskSettings(const TskSettings&) = delete;
    TskSettings& operator=(const TskSettings&) = delete;
    TskSettings(TskSettings&&) = delete;
    TskSettings& operator=(TskSettings&&) = delete;

private:
    TskSettings() = default;

    static TskSettings& get() {
        static TskSettings settings;
        return settings;
    }

    b::cachefile cache { SettingsDirectory() + "settings.json" };
};
