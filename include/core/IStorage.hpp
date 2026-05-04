#pragma once

#include "FlashcardList.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include <memory>

namespace core {
    namespace fs = std::filesystem;

    /**
     * @brief Interface for deck persistence management.
     * Responsibility: Mapping the domain (FlashcardList) to physical storage.
     */
    class IStorage {
    public:
        virtual ~IStorage() = default;

        // --- List Operations ---

        /**
         * @brief Loads a deck from a given relative path.
         */
        virtual std::shared_ptr<FlashcardList> loadList(const fs::path& relativePath) = 0;

        /**
         * @brief Atomically saves a deck to a given relative path.
         * Requirements: Must use a temporary file to ensure data integrity.
         */
        virtual bool saveList(const FlashcardList& list, const fs::path& relativePath) = 0;

        virtual bool deleteList(const fs::path& relativePath) = 0;
        virtual bool moveList(const fs::path& oldPath, const fs::path& newPath) = 0;

        // --- Folder Operations ---
        virtual bool createFolder(const fs::path& folderPath) = 0;
        virtual bool deleteFolder(const fs::path& folderPath) = 0;
        virtual bool renameFolder(const fs::path& oldPath, const fs::path& newPath) = 0;

        // --- Discovery ---
        /**
         * @brief Scans the root data directory for all available .json decks.
         */
        virtual std::vector<fs::path> getAllAvailableLists() const = 0;
    };
}