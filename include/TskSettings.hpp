#pragma once

#include <utility>

#include "battery/core/all.hpp"
#include "battery/core/cache.hpp"
#include "magic_enum.hpp"

enum class TskSetting {
    NONE,
    PROPERTY
};

class TskSettings {
public:
    TskSettings() = delete;

    inline static const size_t MAX_NUMBER_OF_RECENT_FILES = 10;

    static b::fs::path SettingsDirectory() {
        return b::folders::get_config_home();
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
            std::vector<std::string> files = cache["recent_files"];
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

        // If the file already exists, delete and append again
        for (size_t i = 0; i < files.size(); i++) {
            if (files[i] == path) {
                files.erase(files.begin() + i);
                break;
            }
        }

        // Delete first one, if there's too many
        while (files.size() > MAX_NUMBER_OF_RECENT_FILES) {
            files.erase(files.begin());
        }

        files.emplace_back(path);
        cache["recent_files"] = files;
    }

    static nlohmann::json Get(TskSetting setting) {
        return cache[magic_enum::enum_name(setting)];
    }

    static void Set(TskSetting setting, nlohmann::json value) {
        cache[magic_enum::enum_name(setting)] = std::move(value);
    }

private:
    inline static b::cachefile cache { SettingsDirectory() + "settings.json" };   // TODO: Document folders API
};