#include "storage/JsonStorage.hpp"
#include "core/Flashcard.hpp" // Include Flashcard to use its enums and struct
#include "storage/JsonMappers.hpp"

#include <unordered_set>

namespace storage {
    namespace fs = std::filesystem;

    namespace {

    // Helper function to extract duplicate parent directory creation logic
    bool ensureParentDirectoryExists(const fs::path& fullPath) {
        auto parent = fullPath.parent_path();
        if (!parent.empty() && !fs::exists(parent)) {
            return fs::create_directories(parent);
        }
        return true;
    }

    bool cardsNeedNormalization(const nlohmann::json& root) {
        if (!root.contains(core::json_keys::kCards) || !root.at(core::json_keys::kCards).is_array()) {
            return false;
        }

        std::unordered_set<std::string> seenIds;
        for (const auto& card : root.at(core::json_keys::kCards)) {
            if (!card.is_object() || !card.contains(core::json_keys::kId) || !card.at(core::json_keys::kId).is_string()) {
                return true;
            }

            if (!card.contains(core::json_keys::kStateFrontToBack) || !card.contains(core::json_keys::kStateBackToFront)) {
                return true;
            }

            const std::string id = card.at(core::json_keys::kId).get<std::string>();
            if (id.empty()) {
                return true;
            }

            if (!seenIds.insert(id).second) {
                return true;
            }
        }

        return false;
    }

    } // end anonymous namespace

    // --- JsonStorage Implementation ---

    JsonStorage::JsonStorage(fs::path basePath) : _basePath(std::move(basePath)) {
        // Ensure the base path exists
        if (!fs::exists(_basePath)) {
            if (!fs::create_directories(_basePath)) {
                throw std::runtime_error("Failed to create base directory: " + _basePath.string());
            }
        } else { // Path exists
            if (fs::is_regular_file(_basePath)) { // Explicitly check if it's a regular file
                throw std::runtime_error("Base path exists but is a regular file: " + _basePath.string());
            }
            if (!fs::is_directory(_basePath)) { // Then check if it's not a directory (could be symlink, device, etc.)
                throw std::runtime_error("Base path exists but is not a directory: " + _basePath.string());
            }
        }
    }

    fs::path JsonStorage::getFullPath(const fs::path& relativePath) const {
        return _basePath / relativePath;
    }

    fs::path JsonStorage::getTempPath(const fs::path& relativePath) const {
        return getFullPath(relativePath).string() + ".tmp";
    }


    std::shared_ptr<core::FlashcardList> JsonStorage::loadList(const fs::path& relativePath) {
        const fs::path fullPath = getFullPath(relativePath);

        if (!fs::exists(fullPath) || !fs::is_regular_file(fullPath)) {
            return nullptr; // List file does not exist
        }

        std::ifstream ifs(fullPath, std::ios::binary);
        if (!ifs.is_open()) {
            throw std::runtime_error("Failed to open list file for reading: " + fullPath.string());
        }

        try {
            nlohmann::json jso;
            ifs >> jso;

            const bool needsNormalization = cardsNeedNormalization(jso);

            // Create the FlashcardList directly, its name will be set by from_json.
            // Pass a temporary name as the constructor requires one.
            auto flashcardList = std::make_shared<core::FlashcardList>("temp_name_for_deserialization");
            core::from_json(jso, *flashcardList); // Explicit namespace to avoid ADL ambiguity

            if (needsNormalization) {
                if (!saveList(*flashcardList, relativePath)) {
                    throw std::runtime_error("Failed to persist normalized card fields for list: " + fullPath.string());
                }
            }

            return flashcardList;

        } catch (const nlohmann::json::exception& e) {
            throw std::runtime_error("JSON parsing error while loading list '" + fullPath.string() + "': " + e.what());
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Data format error while loading list '" + fullPath.string() + "': " + e.what());
        } catch (const std::exception& e) {
            throw std::runtime_error("An unexpected error occurred loading list '" + fullPath.string() + "': " + e.what());
        }
    }

    bool JsonStorage::saveList(const core::FlashcardList& list, const fs::path& relativePath) {
        const fs::path fullPath = getFullPath(relativePath);
        const fs::path tempPath = getTempPath(relativePath);

        // Ensure parent directory exists for the final and temporary files
        if (!ensureParentDirectoryExists(fullPath)) {
            return false;
        }

        try {
            nlohmann::json jso;
            core::to_json(jso, list); // Explicit namespace to avoid ADL ambiguity

            // Write to a temporary file
            std::ofstream ofs(tempPath, std::ios::binary | std::ios::trunc);
            if (!ofs.is_open()) {
                throw std::runtime_error("Failed to open temporary file for writing: " + tempPath.string());
            }
            ofs << jso.dump(4); // Pretty print with 4 spaces indent

            // Atomically replace the original file
            // std::filesystem::rename is atomic for same-filesystem moves on POSIX systems
            // On Windows, it tries to be atomic, but may fail if the target exists and is in use.
            fs::rename(tempPath, fullPath);
            return true;

        } catch (const nlohmann::json::exception& e) {
            // Clean up temporary file on error
            fs::remove(tempPath);
            throw std::runtime_error("JSON serialization error while saving list '" + fullPath.string() + "': " + e.what());
        } catch (const std::runtime_error& e) {
            fs::remove(tempPath);
            throw std::runtime_error("File operation error while saving list '" + fullPath.string() + "': " + e.what());
        } catch (const std::exception& e) {
            fs::remove(tempPath);
            throw std::runtime_error("An unexpected error occurred saving list '" + fullPath.string() + "': " + e.what());
        }
    }

    bool JsonStorage::deleteList(const fs::path& relativePath) {
        const fs::path fullPath = getFullPath(relativePath);
        if (!fs::is_regular_file(fullPath)) {
            return false;
        }
        std::error_code erc;
        const bool removed = fs::remove(fullPath, erc);
        if (erc) {
            return false;
        }
        return removed;
    }

    bool JsonStorage::moveList(const fs::path& oldPath, const fs::path& newPath) {
        const fs::path fullOldPath = getFullPath(oldPath);
        const fs::path fullNewPath = getFullPath(newPath);

        if (!fs::exists(fullOldPath)) {
            return false; // Source list does not exist
        }
        if (fs::exists(fullNewPath)) {
            return false; // Target list already exists
        }

        // Ensure parent directory for new path exists
        if (!ensureParentDirectoryExists(fullNewPath)) {
            return false;
        }

        std::error_code erc;
        fs::rename(fullOldPath, fullNewPath, erc);
        return !static_cast<bool>(erc);
    }

    bool JsonStorage::createFolder(const fs::path& folderPath) {
        const fs::path fullPath = getFullPath(folderPath);
        std::error_code erc;
        const bool created = fs::create_directories(fullPath, erc);
        if (erc && erc != std::errc::file_exists) { // Ignore error if it's just that it already exists
            return false;
        }
        return created || fs::is_directory(fullPath); // Return true if it was created or already existed
    }

    bool JsonStorage::deleteFolder(const fs::path& folderPath) {
        const fs::path fullPath = getFullPath(folderPath);
        if (!fs::exists(fullPath) || !fs::is_directory(fullPath)) {
            return false; // Not a directory or does not exist
        }

        std::error_code erc;
        const std::uintmax_t removed = fs::remove_all(fullPath, erc);
        if (erc) {
            return false; // Other errors
        }
        return removed > 0;
    }

    bool JsonStorage::renameFolder(const fs::path& oldPath, const fs::path& newPath) {
        const fs::path fullOldPath = getFullPath(oldPath);
        const fs::path fullNewPath = getFullPath(newPath);

        if (!fs::exists(fullOldPath) || !fs::is_directory(fullOldPath)) {
            return false; // Source folder does not exist or is not a directory
        }
        if (fs::exists(fullNewPath)) {
            return false; // Target folder already exists
        }

        // Ensure parent directory for new path exists
        if (!ensureParentDirectoryExists(fullNewPath)) {
            return false;
        }

        std::error_code erc;
        fs::rename(fullOldPath, fullNewPath, erc);
        return !static_cast<bool>(erc);
    }

    std::vector<fs::path> JsonStorage::getAllAvailableLists() const {
        std::vector<fs::path> availableLists;
        if (!fs::exists(_basePath) || !fs::is_directory(_basePath)) {
            return availableLists; // Base path does not exist or is not a directory
        }

        for (const auto& entry : fs::recursive_directory_iterator(_basePath)) {
            if ((entry.is_regular_file() && entry.path().extension() == ".json") || entry.is_directory()) {
                // Get the path relative to _basePath
                availableLists.push_back(fs::relative(entry.path(), _basePath));
            }
        }
        return availableLists;
    }

} // namespace storage
