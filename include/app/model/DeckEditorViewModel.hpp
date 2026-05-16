#pragma once

#include "core/FlashcardList.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <functional>

namespace app::model {

    struct DeckEditorViewModel {
        std::shared_ptr<core::FlashcardList> deck;
        std::map<std::string, bool> cardSelectionState;
        
        // New card buffers
        std::string newFront;
        std::string newBack;

        // Edit card buffers
        std::string editCardId;
        std::string editFront;
        std::string editBack;

        // List buffers for move/copy operations
        std::vector<std::string> availableLists;
        int selectedListIndex = 0;

        void setDeck(std::shared_ptr<core::FlashcardList> newDeck) {
            deck = std::move(newDeck);
            cardSelectionState.clear();
        }

        void setAvailableLists(std::vector<std::string> listNames) {
            availableLists = std::move(listNames);
            selectedListIndex = 0;
        }
        
        void clearNewCardBuffers() {
            newFront.clear();
            newBack.clear();
        }
        
        void startEditing(const std::shared_ptr<core::Flashcard>& card) {
            if (card) {
                editCardId = card->id;
                editFront = card->text_front;
                editBack = card->text_back;
            }
        }
        
        std::vector<std::string> getSelectedCardIds() const {
            std::vector<std::string> ids;
            for (const auto& [id, selected] : cardSelectionState) {
                if (selected) ids.push_back(id);
            }
            return ids;
        }
        
        void clearSelectionFor(const std::vector<std::string>& ids) {
            for (const auto& id : ids) {
                cardSelectionState.erase(id); // Effectively unselects them
            }
        }

        void selectAllCards() {
            for (auto& [id, selected] : cardSelectionState) {
                (void)id;
                selected = true;
            }
        }

        void deselectAllCards() {
            for (auto& [id, selected] : cardSelectionState) {
                (void)id;
                selected = false;
            }
        }
        
        size_t getSelectedCount() const {
            return static_cast<size_t>(std::count_if(
                cardSelectionState.begin(),
                cardSelectionState.end(),
                [](const auto& entry) { return entry.second; }));
        }
    };

} // namespace app::model
