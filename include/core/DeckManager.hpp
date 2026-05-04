#pragma once

#include "FlashcardList.hpp"
#include "IDeckManager.hpp"
#include "IStorage.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace core {

    class DeckManager : public IDeckManager {
    public:
        explicit DeckManager(std::unique_ptr<IStorage> storage);
        ~DeckManager() override = default;

        // Storage Pass-throughs
        std::vector<std::filesystem::path> getAllAvailableLists() const override;
        bool createFolder(const std::filesystem::path& folderPath) override;
        bool deleteFolder(const std::filesystem::path& folderPath) override;
        bool renameFolder(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) override;
        bool moveListFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) override;
        bool deleteListFile(const std::filesystem::path& relativePath) override;

        // List Management
        std::shared_ptr<FlashcardList> loadList(const std::filesystem::path& relativePath) override;
        bool createList(const std::string& listName, const std::filesystem::path& relativePath) override;
        bool createList(const std::string& listName) override; // Memory only
        bool saveList(const std::string& listName) override;
        bool deleteList(const std::string& listName) override;
        std::vector<std::string> getAllListNames() const override;
        std::shared_ptr<FlashcardList> getList(const std::string& listName) const override;

        // Single Card Facade
        bool addCardToList(const std::string& listName, std::shared_ptr<Flashcard> card) override;
        bool removeCardFromList(const std::string& listName, const std::string& cardId) override;
        bool updateCardInList(const std::string& listName, const std::string& cardId, const std::string& newFront, const std::string& newBack) override;
        bool moveCard(const std::string& cardId, const std::string& sourceListName, const std::string& targetListName) override;

        // Bulk Operations Facade
        size_t addCardsToList(const std::string& listName, const std::vector<std::shared_ptr<Flashcard>>& cards) override;
        size_t removeCardsFromList(const std::string& listName, const std::vector<std::string>& cardIds) override;
        size_t moveCards(const std::vector<std::string>& cardIds, const std::string& sourceListName, const std::string& targetListName) override;
        size_t importCardsFromFile(const std::string& listName, const std::filesystem::path& filePath, char delimiter = ',', bool ignoreHeader = true) override;

    private:
        std::unique_ptr<IStorage> _storage;
        std::unordered_map<std::string, std::shared_ptr<FlashcardList>> decks;
        std::unordered_map<std::string, std::filesystem::path> listPaths;
    };

} // namespace core
