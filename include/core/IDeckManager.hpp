#pragma once

#include "FlashcardList.hpp"
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace core {

    class IDeckManager {
    public:
        virtual ~IDeckManager() = default;

        // Storage Pass-throughs
        virtual std::vector<std::filesystem::path> getAllAvailableLists() const = 0;
        virtual bool createFolder(const std::filesystem::path& folderPath) = 0;
        virtual bool deleteFolder(const std::filesystem::path& folderPath) = 0;
        virtual bool renameFolder(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) = 0;
        virtual bool moveListFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) = 0;
        virtual bool deleteListFile(const std::filesystem::path& relativePath) = 0;

        // List Management
        virtual std::shared_ptr<FlashcardList> loadList(const std::filesystem::path& relativePath) = 0;
        virtual bool createList(const std::string& listName, const std::filesystem::path& relativePath) = 0;
        virtual bool createList(const std::string& listName) = 0; 
        virtual bool saveList(const std::string& listName) = 0;
        virtual bool deleteList(const std::string& listName) = 0;
        virtual std::vector<std::string> getAllListNames() const = 0;
        virtual std::shared_ptr<FlashcardList> getList(const std::string& listName) const = 0;

        // Facades
        virtual bool addCardToList(const std::string& listName, std::shared_ptr<Flashcard> card) = 0;
        virtual bool removeCardFromList(const std::string& listName, const std::string& cardId) = 0;
        virtual bool updateCardInList(const std::string& listName, const std::string& cardId, const std::string& newFront, const std::string& newBack) = 0;
        virtual bool moveCard(const std::string& cardId, const std::string& sourceListName, const std::string& targetListName) = 0;
        virtual size_t addCardsToList(const std::string& listName, const std::vector<std::shared_ptr<Flashcard>>& cards) = 0;
        virtual size_t removeCardsFromList(const std::string& listName, const std::vector<std::string>& cardIds) = 0;
        virtual size_t moveCards(const std::vector<std::string>& cardIds, const std::string& sourceListName, const std::string& targetListName) = 0;
        virtual size_t importCardsFromFile(const std::string& listName, const std::filesystem::path& filePath, char delimiter = ',', bool ignoreHeader = true) = 0;
    };

} // namespace core