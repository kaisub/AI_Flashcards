#include "core/DeckManager.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>

namespace core {

    DeckManager::DeckManager(std::unique_ptr<IStorage> storage) : _storage(std::move(storage)) {}

    std::vector<std::filesystem::path> DeckManager::getAllAvailableLists() const {
        return _storage ? _storage->getAllAvailableLists() : std::vector<std::filesystem::path>{};
    }
    bool DeckManager::createFolder(const std::filesystem::path& folderPath) {
        return _storage ? _storage->createFolder(folderPath) : false;
    }
    bool DeckManager::deleteFolder(const std::filesystem::path& folderPath) {
        return _storage ? _storage->deleteFolder(folderPath) : false;
    }
    bool DeckManager::renameFolder(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) {
        return _storage ? _storage->renameFolder(oldPath, newPath) : false;
    }
    bool DeckManager::moveListFile(const std::filesystem::path& oldPath, const std::filesystem::path& newPath) {
        return _storage ? _storage->moveList(oldPath, newPath) : false;
    }
    bool DeckManager::deleteListFile(const std::filesystem::path& relativePath) {
        return _storage ? _storage->deleteList(relativePath) : false;
    }

    std::shared_ptr<FlashcardList> DeckManager::loadList(const std::filesystem::path& relativePath) {
        if (!_storage) { return nullptr; }
        auto list = _storage->loadList(relativePath);
        if (list) {
            decks[list->getName()] = list;
            listPaths[list->getName()] = relativePath;
        }
        return list;
    }

    bool DeckManager::createList(const std::string& listName, const std::filesystem::path& relativePath) {
        if (decks.contains(listName)) { return false; }
        auto list = std::make_shared<FlashcardList>(listName);
        if (_storage && _storage->saveList(*list, relativePath)) {
            decks[listName] = list;
            listPaths[listName] = relativePath;
            return true;
        }
        return false;
    }

    bool DeckManager::createList(const std::string& listName) {
        if (decks.contains(listName)) {
            return false;
        }
        decks[listName] = std::make_shared<FlashcardList>(listName);
        return true;
    }

    bool DeckManager::saveList(const std::string& listName) {
        auto list = getList(listName);
        if (list && listPaths.contains(listName) && _storage) {
            return _storage->saveList(*list, listPaths.at(listName));
        }
        return false;
    }

    bool DeckManager::deleteList(const std::string& listName) {
        listPaths.erase(listName);
        return decks.erase(listName) > 0;
    }

    std::vector<std::string> DeckManager::getAllListNames() const {
        std::vector<std::string> names;
        names.reserve(decks.size());
        for (const auto& [name, list] : decks) {
            names.push_back(name);
        }
        std::sort(names.begin(), names.end());
        return names;
    }

    std::shared_ptr<FlashcardList> DeckManager::getList(const std::string& listName) const {
        auto it = decks.find(listName);
        if (it == decks.end()) {
            return nullptr;
        }
        return it->second;
    }

    bool DeckManager::addCardToList(const std::string& listName, std::shared_ptr<Flashcard> card) {
        if (auto list = getList(listName)) {
            if (list->addCard(card)) {
                saveList(listName);
                return true;
            }
        }
        return false;
    }

    bool DeckManager::removeCardFromList(const std::string& listName, const std::string& cardId) {
        if (auto list = getList(listName)) {
            if (list->removeCard(cardId)) {
                saveList(listName);
                return true;
            }
        }
        return false;
    }

    bool DeckManager::updateCardInList(const std::string& listName, const std::string& cardId, const std::string& newFront, const std::string& newBack) {
        if (auto list = getList(listName)) {
            if (list->updateCard(cardId, newFront, newBack)) {
                saveList(listName);
                return true;
            }
        }
        return false;
    }

    bool DeckManager::moveCard(const std::string& cardId, const std::string& sourceListName, const std::string& targetListName) {
        if (sourceListName == targetListName) {
            return false;
        }

        auto sourceList = getList(sourceListName);
        auto targetList = getList(targetListName);

        if (!sourceList || !targetList) {
            return false;
        }

        const std::shared_ptr<Flashcard> cardToMove = sourceList->getCard(cardId);
        if (!cardToMove) {
            return false;
        }

        if (targetList->addCard(cardToMove)) {
            sourceList->removeCard(cardId);
            saveList(sourceListName);
            saveList(targetListName);
            return true;
        }
        return false;
    }

    size_t DeckManager::addCardsToList(const std::string& listName, const std::vector<std::shared_ptr<Flashcard>>& cards) {
        if (auto list = getList(listName)) {
            const size_t added = list->importCardsFrom(cards);
            if (added > 0) { saveList(listName); }
            return added;
        }
        return 0;
    }

    size_t DeckManager::removeCardsFromList(const std::string& listName, const std::vector<std::string>& cardIds) {
        if (auto list = getList(listName)) {
            const size_t removed = list->removeCards(cardIds);
            if (removed > 0) { saveList(listName); }
            return removed;
        }
        return 0;
    }

    size_t DeckManager::moveCards(const std::vector<std::string>& cardIds, const std::string& sourceListName, const std::string& targetListName) {
        if (sourceListName == targetListName) {
            return 0;
        }

        auto sourceList = getList(sourceListName);
        auto targetList = getList(targetListName);

        if (!sourceList || !targetList) {
            return 0;
        }

        size_t movedCount = 0;
        for (const std::string& cardId : cardIds) {
            const std::shared_ptr<Flashcard> cardToMove = sourceList->getCard(cardId);
            if (cardToMove) {
                if (targetList->addCard(cardToMove)) {
                    sourceList->removeCard(cardId);
                    movedCount++;
                }
            }
        }
        if (movedCount > 0) {
            saveList(sourceListName);
            saveList(targetListName);
        }
        return movedCount;
    }

    size_t DeckManager::importCardsFromFile(const std::string& listName, const std::filesystem::path& filePath, char delimiter, bool ignoreHeader) {
        auto list = getList(listName);
        if (!list) {
            return 0;
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file.");
        }

        // Check and remove UTF-8 BOM if present
        char bom[3] = {0};
        file.read(bom, 3);
        if (!(bom[0] == '\xEF' && bom[1] == '\xBB' && bom[2] == '\xBF')) {
            file.seekg(0);
            file.clear(); // Clear EOF or fail bits if file was very small
        }

        std::vector<std::shared_ptr<Flashcard>> newCards;
        std::string line;
        bool firstLine = true;

        auto trim = [](std::string& str) {
            auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
                return std::isspace(ch);
            });
            auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
                return std::isspace(ch);
            }).base();
            str = (start < end) ? std::string(start, end) : std::string();
        };

        while (std::getline(file, line)) {
            if (firstLine && ignoreHeader) {
                firstLine = false;
                continue;
            }
            firstLine = false;

            if (line.empty()) {
                continue;
            }

            std::string front;
            std::string back;

            size_t pos = line.find(delimiter);
            if (pos != std::string::npos) {
                front = line.substr(0, pos);
                back = line.substr(pos + 1);
            } else {
                front = line;
                back = "";
            }

            trim(front);
            trim(back);

            if (front.empty()) {
                continue;
            }

            static size_t cardIdCounter = 0;
            std::string cardId = "import_" + std::to_string(++cardIdCounter) + "_" + std::to_string(std::hash<std::string>{}(front + back));

            auto card = std::make_shared<Flashcard>();
            card->id = cardId;
            card->text_front = front;
            card->text_back = back;
            card->state_Front_to_Back = CardState::New;
            card->state_Back_to_Front = CardState::New;

            newCards.push_back(card);
        }

        file.close();

        return addCardsToList(listName, newCards);
    }

} // namespace core
