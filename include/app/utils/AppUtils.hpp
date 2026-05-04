#pragma once

#include "core/IDeckManager.hpp"
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <filesystem>

namespace app::utils {

    inline std::string generateUniqueCardId() {
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        static std::mt19937 rng{std::random_device{}()};
        return "card-" + std::to_string(timestamp) + "-" + std::to_string(rng());
    }

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