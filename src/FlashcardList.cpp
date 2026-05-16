#include "core/FlashcardList.hpp"

#include <algorithm> // For std::find_if, etc.

namespace core {

    bool FlashcardList::addCard(const std::shared_ptr<Flashcard>& card) {
        if (!card) {
            return false; // Card is null
        }
        auto [itr, inserted] = cards.try_emplace(card->card_id, card);
        if (inserted) {
            cardOrder.push_back(card->card_id);
        }
        return inserted;
    }

    bool FlashcardList::removeCard(const std::string& idx) {
        if (cards.erase(idx) == 0) {
            return false;
        }
        cardOrder.erase(std::remove(cardOrder.begin(), cardOrder.end(), idx), cardOrder.end());
        return true;
    }

    bool FlashcardList::updateCard(const std::string& idx, const std::string& newFront, const std::string& newBack) {
        auto itr = cards.find(idx);
        if (itr == cards.end()) {
            return false; // Card not found
        }
        itr->second->text_front = newFront;
        itr->second->text_back = newBack;
        return true;
    }

    std::shared_ptr<Flashcard> FlashcardList::getCard(const std::string& idx) const {
        auto itr = cards.find(idx);
        if (itr == cards.end()) {
            return nullptr; // Card not found
        }
        return itr->second;
    }

    std::vector<std::shared_ptr<Flashcard>> FlashcardList::getAllCards() const {
        std::vector<std::shared_ptr<Flashcard>> allCards;
        allCards.reserve(cardOrder.size());
        for (const auto& id : cardOrder) {
            auto itr = cards.find(id);
            if (itr != cards.end()) {
                allCards.push_back(itr->second);
            }
        }
        return allCards;
    }

    size_t FlashcardList::removeCards(const std::vector<std::string>& ids) {
        size_t removedCount = 0;
        for (const std::string& idx : ids) {
            if (removeCard(idx)) {
                removedCount++;
            }
        }
        return removedCount;
    }

    size_t FlashcardList::importCardsFrom(const FlashcardList& sourceList) {
        size_t addedCount = 0;
        for (const auto& card_ptr : sourceList.getAllCards()) {
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
        cardOrder.clear();
    }

    size_t FlashcardList::size() const {
        return cards.size();
    }

} // namespace core