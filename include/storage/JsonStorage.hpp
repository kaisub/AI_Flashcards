#pragma once

#include "core/IStorage.hpp"
#include "core/Flashcard.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <memory>
#include <vector>
#include <string>

namespace storage {
    namespace fs = std::filesystem;

    /**
     * @brief Implementation of IStorage interface for JSON persistence using nlohmann/json.
     * Handles serialization and deserialization of FlashcardList objects to/from JSON files.
     * Ensures atomic writes to prevent data corruption.
     */
    class JsonStorage : public core::IStorage {
    public:
        /**
         * @brief Constructs a JsonStorage instance with a specified base path for data.
         * The base path is where all lists and folders will be managed relative to.
         * If the base path does not exist, it will attempt to create it.
         * @param basePath The root directory for storing flashcard data.
         */
        explicit JsonStorage(fs::path basePath);
        
        ~JsonStorage() override = default;

        // --- List Operations ---
        std::shared_ptr<core::FlashcardList> loadList(const fs::path& relativePath) override;
        bool saveList(const core::FlashcardList& list, const fs::path& relativePath) override;
        bool deleteList(const fs::path& relativePath) override;
        bool moveList(const fs::path& oldPath, const fs::path& newPath) override;

        // --- Folder Operations ---
        bool createFolder(const fs::path& folderPath) override;
        bool deleteFolder(const fs::path& folderPath) override;
        bool renameFolder(const fs::path& oldPath, const fs::path& newPath) override;

        // --- Discovery ---
        std::vector<fs::path> getAllAvailableLists() const override;

    private:
        fs::path _basePath;

        fs::path getFullPath(const fs::path& relativePath) const;
        fs::path getTempPath(const fs::path& relativePath) const;
    };

} // namespace storage
