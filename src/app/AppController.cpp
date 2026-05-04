#include "app/AppController.hpp"
#include "app/AppState.hpp"
#include "app/presenters/DeckEditorPresenter.hpp"
#include "app/presenters/ListsBrowserPresenter.hpp"
#include "app/presenters/StudyConfigPresenter.hpp"
#include "app/presenters/StudySessionPresenter.hpp"
#include "app/utils/AppUtils.hpp"
#include "app/views/IDeckEditorView.hpp"
#include "app/views/IListsBrowserView.hpp"
#include "app/views/IStudyConfigView.hpp"
#include "app/views/IStudySessionView.hpp"
#include "core/IDeckManager.hpp"
#include "core/StudySession.hpp"

#include <algorithm>

namespace app {

    /**
     * @brief Constructs an AppController instance.
     * @param deckManager Unique pointer to the deck manager.
     */
    AppController::AppController(std::unique_ptr<core::IDeckManager> deckManager)
    : _deckManager(std::move(deckManager))
    {
        // Any initial setup for the application can go here.
    }

    AppController::~AppController() = default;

    /**
     * @brief Entry point of the application logic. Manages the state machine.
     * The application continues to run until the state transitions to AppState::Exit.
     */
    void AppController::run() {
        while (_currentState != AppState::Exit) {
            switch (_currentState) {
                case AppState::ListsBrowser:
                    handleListsBrowser();
                    break;
                case AppState::DeckEditor:
                    handleDeckEditor();
                    break;
                case AppState::StudyConfig:
                    handleStudyConfig();
                    break;
                case AppState::StudySession:
                    handleStudySession();
                    break;
                case AppState::Settings:
                    // Currently unhandled; transition to Exit.
                    transitionTo(AppState::Exit);
                    break;
                case AppState::Exit:
                    // This state should be handled by the loop condition.
                    break;
            }
        }
    }

    /**
     * @brief Transitions the application to a new state.
     * @param newState The state to transition to.
     */
    void AppController::transitionTo(AppState newState) {
        _currentState = newState;
    }

    void AppController::setListsBrowserView(std::shared_ptr<IListsBrowserView> view) {
        _browserPresenter = std::make_unique<presenters::ListsBrowserPresenter>(std::move(view), _deckManager.get());

        _browserPresenter->onListSelected = [this](const std::filesystem::path& path) {
            _currentListPath = path;
            auto list = _deckManager->loadList(_currentListPath);
            if (list) {
                _currentListName = list->getName();
                transitionTo(AppState::DeckEditor);
            } else {
                // TODO(unknown): Handle failure to load list (e.g., notify the browser presenter to show an error dialog)
            }
        };

        _browserPresenter->onExitAppRequested = [this]() {
            transitionTo(AppState::Exit);
        };
    }

    void AppController::setStudySessionView(std::shared_ptr<IStudySessionView> view) {
        _studySessionPresenter = std::make_unique<presenters::StudySessionPresenter>(std::move(view), _deckManager.get());

        _studySessionPresenter->onExitRequested = [this]() { transitionTo(AppState::StudyConfig); };
    }

    void AppController::setDeckEditorView(std::shared_ptr<IDeckEditorView> view) {
        _deckEditorPresenter = std::make_unique<presenters::DeckEditorPresenter>(std::move(view), _deckManager.get());

        _deckEditorPresenter->onStartStudy = [this]() { transitionTo(AppState::StudyConfig); };
        _deckEditorPresenter->onExitToBrowser = [this]() { transitionTo(AppState::ListsBrowser); };
    }

    void AppController::setStudyConfigView(std::shared_ptr<IStudyConfigView> view) {
        _configPresenter = std::make_unique<presenters::StudyConfigPresenter>(std::move(view));

        _configPresenter->onStart = [this](core::SessionConfig config) {
            _studySessionPresenter->start(_currentListName, _currentListPath, config);
            transitionTo(AppState::StudySession);
        };

        _configPresenter->onCancel = [this]() {
            transitionTo(AppState::DeckEditor);
        };
    }

    /**
     * @brief Handles the logic for the ListsBrowser state.
     * This state is responsible for displaying and navigating available flashcard lists and folders.
     */
    void AppController::handleListsBrowser() {
        if (!_browserPresenter) { transitionTo(AppState::Exit); return; }
        _browserPresenter->start();
    }

    /**
     * @brief Handles the logic for the DeckEditor state.
     * This state is responsible for managing flashcards within a selected list.
     */
    void AppController::handleDeckEditor() {
        if (!_deckEditorPresenter) { transitionTo(AppState::Exit); return; }
        _deckEditorPresenter->start(_currentListName, _currentListPath);
    }

    /**
     * @brief Handles the logic for the StudyConfig state.
     * This state is responsible for configuring session parameters before studying.
     */
    void AppController::handleStudyConfig() {
        if (!_configPresenter) { transitionTo(AppState::Exit); return; }
        _configPresenter->start();
    }

    /**
     * @brief Handles the logic for the StudySession state.
     * This state is responsible for the active learning process.
     */
    void AppController::handleStudySession() {
        if (!_studySessionPresenter) { transitionTo(AppState::Exit); return; }
        _studySessionPresenter->handleStep();
    }

} // namespace app
