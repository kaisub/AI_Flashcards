#include "app/presenters/DeckEditorPresenter.hpp"
#include "app/utils/AppUtils.hpp"
#include <iostream>

namespace app::presenters {

    DeckEditorPresenter::DeckEditorPresenter(std::shared_ptr<IDeckEditorView> view, core::IDeckManager* deckManager)
        : _view(std::move(view)), _deckManager(deckManager) {
        bindEvents();
    }

    void DeckEditorPresenter::start(const std::string& listName, const std::filesystem::path& listPath) {
        _currentListName = listName;
        _currentListPath = listPath;
        
        auto list = _deckManager->loadList(_currentListPath);
        if (!list) {
            return;
        }
        _currentListName = list->getName();

        auto filteredPaths = utils::getOtherAvailableListPaths(_deckManager, _currentListPath);
        std::vector<std::string> availableListNames;
        availableListNames.reserve(filteredPaths.size());
        for (const auto& path : filteredPaths) { availableListNames.push_back(path.string()); }

        _view->setAvailableLists(availableListNames);
        _view->setDeck(list);
        _view->run();
    }

    void DeckEditorPresenter::bindEvents() {
        _view->setOnAddCard([this](const std::string& front, const std::string& back) {
            auto list = _deckManager->getList(_currentListName);
            if (!list) {
                return;
            }
            auto newCard = std::make_shared<core::Flashcard>();
            newCard->card_id = utils::generateUniqueCardId();
            newCard->text_front = front;
            newCard->text_back = back;
            if (_deckManager->addCardToList(list->getName(), newCard)) {
                _deckManager->saveList(list->getName());
                _view->setDeck(list);
            }
        });

        _view->setOnRemoveCard([this](const std::string& cardId) {
            auto list = _deckManager->getList(_currentListName);
            if (!list) {
                return;
            }
            if (_deckManager->removeCardFromList(list->getName(), cardId)) {
                _deckManager->saveList(list->getName());
                _view->setDeck(list);
            }
        });

        _view->setOnUpdateCard([this](const std::string& cardId, const std::string& newFront, const std::string& newBack) {
            auto list = _deckManager->getList(_currentListName);
            if (!list) {
                return;
            }
            if (_deckManager->updateCardInList(list->getName(), cardId, newFront, newBack)) {
                _deckManager->saveList(list->getName());
                _view->setDeck(list);
            }
        });

        _view->setOnDeleteSelected([this](const std::vector<std::string>& cardIds) {
            auto list = _deckManager->getList(_currentListName);
            if (!list) {
                return;
            }
            if (_deckManager->removeCardsFromList(list->getName(), cardIds) > 0) {
                _deckManager->saveList(list->getName());
                _view->setDeck(list);
            }
        });

        _view->setOnMoveSelected([this](const std::vector<std::string>& cardIds, int targetListIndex) {
            auto sourceList = _deckManager->getList(_currentListName);
            if (!sourceList) {
                return;
            }
            auto filteredPaths = utils::getOtherAvailableListPaths(_deckManager, _currentListPath);
            if (targetListIndex < 0 || targetListIndex >= static_cast<int>(filteredPaths.size())) {
                return;
            }
            auto targetList = _deckManager->loadList(filteredPaths[static_cast<size_t>(targetListIndex)]);
            if (!targetList) {
                return;
            }
            if (_deckManager->moveCards(cardIds, sourceList->getName(), targetList->getName()) > 0) {
                _deckManager->saveList(sourceList->getName());
                _deckManager->saveList(targetList->getName());
                _view->setDeck(sourceList);
            }
        });

        _view->setOnCopySelected([this](const std::vector<std::string>& cardIds, int targetListIndex) {
            auto sourceList = _deckManager->getList(_currentListName);
            if (!sourceList) {
                return;
            }
            auto filteredPaths = utils::getOtherAvailableListPaths(_deckManager, _currentListPath);
            if (targetListIndex < 0 || targetListIndex >= static_cast<int>(filteredPaths.size())) {
                return;
            }
            auto targetList = _deckManager->loadList(filteredPaths[static_cast<size_t>(targetListIndex)]);
            if (!targetList) {
                return;
            }

            std::vector<std::shared_ptr<core::Flashcard>> copiedCards;
            for (const auto& cardId : cardIds) {
                auto card = sourceList->getCard(cardId);
                if (!card) {
                    continue;
                }
                auto copiedCard = std::make_shared<core::Flashcard>(
                    core::Flashcard::makeCloneAsNew(*card, utils::generateUniqueCardId()));
                copiedCards.push_back(copiedCard);
            }
            if (_deckManager->addCardsToList(targetList->getName(), copiedCards) > 0) {
                _deckManager->saveList(targetList->getName());
            }
        });

        _view->setOnImportRequested([this](const std::string& path, char delim, bool ignoreHeader) {
            auto list = _deckManager->getList(_currentListName);
            if (!list) {
                return;
            }
            try {
                _deckManager->importCardsFromFile(list->getName(), path, delim, ignoreHeader);
                _deckManager->saveList(list->getName());
                _view->setDeck(list);
            } catch (...) {
                std::cerr << "setOnImportRequested exception occurred." << '\n';
            }
        });

        _view->setOnStartStudy([this]() {
            auto list = _deckManager->getList(_currentListName);
            if (!list || list->getAllCards().empty()) {
                return;
            }
            _deckManager->saveList(list->getName()); // Ensure state is firmly committed
            if (onStartStudy) {
                onStartStudy();
            }
        });

        _view->setOnExitToBrowser([this]() {
            _deckManager->saveList(_currentListName); // Safety net for learning state
            if (onExitToBrowser) {
                onExitToBrowser();
            }
        });
    }

} // namespace app::presenters
