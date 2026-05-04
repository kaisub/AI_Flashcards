#pragma once
#include "core/IDeckManager.hpp"
#include "core/IStorage.hpp"
#include "core/StudySession.hpp"
#include "app/AppState.hpp"
#include <memory>
#include <filesystem>
#include <optional>
#include <vector>

namespace app {
    // Forward declarations of interfaces
    class IListsBrowserView;
    class IDeckEditorView;
    class IStudySessionView;
    class IStudyConfigView;

    namespace presenters {
        class ListsBrowserPresenter;
        class DeckEditorPresenter;
        class StudyConfigPresenter;
        class StudySessionPresenter;
    }

    class AppControllerAccess;

    class AppController {
    public:
        explicit AppController(std::unique_ptr<core::IDeckManager> deckManager);
        ~AppController();

        /**
         * @brief Entry point of the application logic.
         */
        void run();

        /**
         * @brief Transitions the application to a new state.
         */
        AppState getCurrentState() const { return _currentState; }

        void setListsBrowserView(std::shared_ptr<IListsBrowserView> view);
        void setStudySessionView(std::shared_ptr<IStudySessionView> view);
        void setDeckEditorView(std::shared_ptr<IDeckEditorView> view);
        void setStudyConfigView(std::shared_ptr<IStudyConfigView> view);

    private:
        friend class AppControllerAccess;

        void transitionTo(AppState newState);
        void handleListsBrowser();
        void handleDeckEditor();
        void handleStudyConfig();
        void handleStudySession();

        std::unique_ptr<core::IDeckManager> _deckManager;
        std::unique_ptr<presenters::ListsBrowserPresenter> _browserPresenter;
        std::unique_ptr<presenters::StudySessionPresenter> _studySessionPresenter;
        std::unique_ptr<presenters::DeckEditorPresenter> _deckEditorPresenter;
        std::unique_ptr<presenters::StudyConfigPresenter> _configPresenter;

        AppState _currentState = AppState::ListsBrowser;
        std::filesystem::path _currentListPath;
        std::string _currentListName; // tracks the logical list name stored in DeckManager
    };
}
