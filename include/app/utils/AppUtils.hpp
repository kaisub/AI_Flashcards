#pragma once

#include "core/FileTypeConstants.hpp"
#include "core/IDeckManager.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace app::utils {

    inline std::vector<std::filesystem::path> getOtherAvailableListPaths(const core::IDeckManager* deckManager, const std::filesystem::path& currentPath) {
        std::vector<std::filesystem::path> filteredPaths;
        for (const auto& path : deckManager->getAllAvailableLists()) {
            if (path != currentPath && path.extension() == core::constants::kJsonExtension) {
                filteredPaths.push_back(path);
            }
        }
        return filteredPaths;
    }

    inline std::string ensureJsonExtension(const std::string& name) {
        return name.ends_with(core::constants::kJsonExtension) ? name : name + std::string(core::constants::kJsonExtension);
    }
} // namespace app::utils