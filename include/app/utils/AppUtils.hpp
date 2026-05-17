#pragma once

#include "core/IDeckManager.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace app::utils {

    inline std::vector<std::filesystem::path> getOtherAvailableListPaths(const core::IDeckManager* deckManager, const std::filesystem::path& currentPath) {
        std::vector<std::filesystem::path> filteredPaths;
        for (const auto& path : deckManager->getAllAvailableLists()) {
            if (path != currentPath && path.extension() == ".json") {
                filteredPaths.push_back(path);
            }
        }
        return filteredPaths;
    }

    inline std::string ensureJsonExtension(const std::string& name) {
        return name.ends_with(".json") ? name : name + ".json";
    }
} // namespace app::utils