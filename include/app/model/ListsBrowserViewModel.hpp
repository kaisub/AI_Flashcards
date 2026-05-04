#pragma once

#include "app/model/BrowserItem.hpp"
#include <filesystem>
#include <vector>
#include <string>

namespace app::model {

    struct ListsBrowserViewModel {
        std::filesystem::path currentPath;
        std::vector<app::model::BrowserItem> items;
        
        int selectedIndex = 0;
        std::string pendingSelection;
        std::string inputBuffer;

        // Helper to safely strip the .json extension for UI display
        static void stripJsonExtension(std::string& name) {
            if (name.ends_with(".json")) {
                name.erase(name.length() - 5);
            }
        }

        void updateList(const std::filesystem::path& path, const std::vector<app::model::BrowserItem>& newItems) {
            // Detect if we navigated back to a parent directory so we can highlight the folder we just left
            if (!currentPath.empty() && currentPath.parent_path() == path) {
                pendingSelection = currentPath.filename().string();
            }
            
            currentPath = path;
            items = newItems;
            
            // Preserve selection index or select newly created/renamed item
            bool selection_updated = false;
            if (!pendingSelection.empty()) {
                for (size_t i = 0; i < items.size(); ++i) {
                    std::string baseName = items[i].displayName;
                    if (!items[i].isDirectory) {
                        stripJsonExtension(baseName);
                    }
                    if (baseName == pendingSelection || items[i].displayName == pendingSelection) {
                        selectedIndex = static_cast<int>(i);
                        selection_updated = true;
                        break;
                    }
                }
                pendingSelection = ""; // Clear it so it only applies once
            }

            if (!selection_updated) {
                if (items.empty()) {
                    selectedIndex = 0;
                } else if (selectedIndex >= static_cast<int>(items.size())) {
                    selectedIndex = static_cast<int>(items.size()) - 1;
                }
            }
        }

        void prepareRename() {
            if (hasValidSelection()) {
                inputBuffer = items[selectedIndex].displayName;
                if (!items[selectedIndex].isDirectory) {
                    stripJsonExtension(inputBuffer);
                }
            }
        }

        bool hasValidSelection() const {
            return !items.empty() && selectedIndex >= 0 && selectedIndex < static_cast<int>(items.size());
        }

        const app::model::BrowserItem& getSelectedItem() const {
            return items[selectedIndex];
        }
    };

} // namespace app::model