#pragma once

#include "app/views/IStudySessionView.hpp"
#include "core/StudySession.hpp"
#include "core/IDeckManager.hpp"
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace app::presenters {

    class StudySessionPresenter {
    public:
        StudySessionPresenter(std::shared_ptr<IStudySessionView> view, core::IDeckManager* deckManager);

        void start(const std::string& listName, const std::filesystem::path& listPath, const core::SessionConfig& config);
        void handleStep(); // Processes the next card or completes the session

        std::function<void()> onExitRequested;

    private:
        std::shared_ptr<IStudySessionView> _view;
        core::IDeckManager* _deckManager;
        std::unique_ptr<core::StudySession> _session;
        std::string _currentListName;
        std::filesystem::path _currentListPath;
        std::optional<core::ReviewItem> _currentReviewItem;
        std::vector<std::filesystem::path> _availableListPaths;

        void bindEvents();
    };
} // namespace app::presenters