#include "app/presenters/StudySessionPresenter.hpp"
#include "app/utils/AppUtils.hpp"
#include "core/CardId.hpp"

namespace app::presenters {

    StudySessionPresenter::StudySessionPresenter(std::shared_ptr<IStudySessionView> view, core::IDeckManager* deckManager)
        : _view(std::move(view)), _deckManager(deckManager) {
        bindEvents();
    }

    void StudySessionPresenter::start(const std::string& listName, const std::filesystem::path& listPath, const core::SessionConfig& config) {
        _currentListName = listName;
        _currentListPath = listPath;
        _currentConfig = config;
        
        auto list = _deckManager->loadList(_currentListPath);
        if (!list || list->getAllCards().empty()) {
            if (onExitRequested) {
                onExitRequested();
            }
            return;
        }
        _currentListName = list->getName();
        
        _session = std::make_unique<core::StudySession>(list->getAllCards(), config);
        
        _availableListPaths = utils::getOtherAvailableListPaths(_deckManager, _currentListPath);
        std::vector<std::string> listDisplayNames;
        listDisplayNames.reserve(_availableListPaths.size());
        for (const auto& path : _availableListPaths) { listDisplayNames.push_back(path.string()); }
        
        _view->setAvailableLists(listDisplayNames);
    }

    void StudySessionPresenter::handleStep() {
        if (!_session) {
            if (onExitRequested) {
                onExitRequested();
            }
            return;
        }
        
        _currentReviewItem = _session->getNextItem();
        if (_currentReviewItem) {
            _view->showCard(*_currentReviewItem->card, false);
        } else {
            _view->showSessionComplete();
        }
        _view->run();
    }

    void StudySessionPresenter::bindEvents() {
        _view->setOnCardRated([this](core::CardState newState) {
            if (_session && _currentReviewItem) {
                _session->submitAnswer(*_currentReviewItem, newState);
            }
        });

        _view->setOnUndoRequested([this]() {
            if (_session) {
                _session->undoLastAction();
            }
        });

        _view->setOnResetAllRequested([this]() {
            auto list = _deckManager->getList(_currentListName);
            if (!list) {
                return;
            }

            for (const auto& card : list->getAllCards()) {
                if (!card) {
                    continue;
                }
                card->state_Front_to_Back = core::CardState::New;
                card->state_Back_to_Front = core::CardState::New;
            }

            _deckManager->saveList(_currentListName);
            _session = std::make_unique<core::StudySession>(list->getAllCards(), _currentConfig);
            _currentReviewItem.reset();
        });

        _view->setOnExitRequested([this]() {
            _deckManager->saveList(_currentListName);
            if (onExitRequested) {
                onExitRequested();
            }
        });
        
        _view->setOnEditRequested([this](const std::string& newFront, const std::string& newBack) {
            if (!_currentReviewItem || !_currentReviewItem->card) {
                return;
            }
            if (_deckManager->updateCardInList(_currentListName, _currentReviewItem->card->card_id, newFront, newBack)) {
                _deckManager->saveList(_currentListName);
                _view->showCard(*_currentReviewItem->card, false);
            }
        });
        
        _view->setOnDeleteRequested([this]() {
            if (!_currentReviewItem || !_currentReviewItem->card) {
                return;
            }
            const std::string cardId = _currentReviewItem->card->card_id;
            if (_deckManager->removeCardFromList(_currentListName, cardId)) {
                _deckManager->saveList(_currentListName);
                _session->removeCardFromSession(cardId);
                _currentReviewItem.reset();
            }
        });
        
        _view->setOnCopyRequested([this](int selectedListIndex) {
            if (!_currentReviewItem || !_currentReviewItem->card) {
                return;
            }

            auto freshAvailableListPaths = utils::getOtherAvailableListPaths(_deckManager, _currentListPath);
            if (selectedListIndex < 0 || selectedListIndex >= static_cast<int>(freshAvailableListPaths.size())) {
                return;
            }

            auto targetList = _deckManager->loadList(freshAvailableListPaths[static_cast<size_t>(selectedListIndex)]);
            if (!targetList) {
                return;
            }

            auto copiedCard = std::make_shared<core::Flashcard>(
                core::Flashcard::makeCloneAsNew(*_currentReviewItem->card, core::generateUniqueCardId()));

            if (_deckManager->addCardToList(targetList->getName(), copiedCard)) {
                _deckManager->saveList(targetList->getName());
            }
        });
    }
} // namespace app::presenters