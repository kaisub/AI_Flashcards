#include "core/FlashcardList.hpp"

#include <algorithm> // For std::find_if, etc.

namespace core {

    bool FlashcardList::addCard(const std::shared_ptr<Flashcard>& card) {
        if (!card) {
            return false; // Card is null
        }
        auto [it, inserted] = cards.try_emplace(card->id, card);
        return inserted;
    }

    bool FlashcardList::removeCard(const std::string& id) {
        return cards.erase(id) > 0;
    }

    bool FlashcardList::updateCard(const std::string& id, const std::string& newFront, const std::string& newBack) {
        auto it = cards.find(id);
        if (it == cards.end()) {
            return false; // Card not found
        }
        it->second->text_front = newFront;
        it->second->text_back = newBack;
        return true;
    }

    std::shared_ptr<Flashcard> FlashcardList::getCard(const std::string& id) const {
        auto it = cards.find(id);
        if (it == cards.end()) {
            return nullptr; // Card not found
        }
        return it->second;
    }

    std::vector<std::shared_ptr<Flashcard>> FlashcardList::getAllCards() const {
        std::vector<std::shared_ptr<Flashcard>> allCards;
        allCards.reserve(cards.size());
    for (const auto& [id, card_ptr] : cards) {
        allCards.push_back(card_ptr);
        }
        return allCards;
    }

    size_t FlashcardList::removeCards(const std::vector<std::string>& ids) {
        size_t removedCount = 0;
        for (const std::string& id : ids) {
            if (removeCard(id)) {
                removedCount++;
            }
        }
        return removedCount;
    }

    size_t FlashcardList::importCardsFrom(const FlashcardList& sourceList) {
        size_t addedCount = 0;
    for (const auto& [id, card_ptr] : sourceList.cards) {
            // Create a new shared_ptr pointing to the same Flashcard object
            // This ensures both lists share ownership of the same card instance,
            // which is crucial for StudySession and state updates.
        if (addCard(card_ptr)) {
                addedCount++;
            }
        }
        return addedCount;
    }

    size_t FlashcardList::importCardsFrom(const std::vector<std::shared_ptr<Flashcard>>& newCards) {
        size_t addedCount = 0;
        for (const auto& card : newCards) {
            if (addCard(card)) {
                addedCount++;
            }
        }
        return addedCount;
    }

    void FlashcardList::clear() {
        cards.clear();
    }

    size_t FlashcardList::size() const {
        return cards.size();
    }

} // namespace core