#pragma once

#include "Flashcard.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

namespace core {

    class FlashcardList {
    public:
        explicit FlashcardList(std::string name) : listName(std::move(name)) {}

        const std::string& getName() const { return listName; }
        void setName(const std::string& newName) { listName = newName; }

        // Basic CRUD
        bool addCard(const std::shared_ptr<Flashcard>& card);
        bool removeCard(const std::string& id);
        bool updateCard(const std::string& id, const std::string& newFront, const std::string& newBack);
        
        std::shared_ptr<Flashcard> getCard(const std::string& id) const;
        std::vector<std::shared_ptr<Flashcard>> getAllCards() const;

        // Bulk Operations
        size_t removeCards(const std::vector<std::string>& ids);
        size_t importCardsFrom(const FlashcardList& sourceList);
        size_t importCardsFrom(const std::vector<std::shared_ptr<Flashcard>>& newCards);

        void clear();
        size_t size() const;

    private:
        std::string listName;
        std::unordered_map<std::string, std::shared_ptr<Flashcard>> cards;
    };

} // namespace core