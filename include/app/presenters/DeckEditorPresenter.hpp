#pragma once

#include "app/views/IDeckEditorView.hpp"
#include "core/IDeckManager.hpp"
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace app::presenters {

    class DeckEditorPresenter {
    public:
        DeckEditorPresenter(std::shared_ptr<IDeckEditorView> view, core::IDeckManager* deckManager);

        void start(const std::string& listName, const std::filesystem::path& listPath);

        std::function<void()> onStartStudy;
        std::function<void()> onExitToBrowser;

    private:
        std::shared_ptr<IDeckEditorView> _view;
        core::IDeckManager* _deckManager;
        std::filesystem::path _currentListPath;
        std::string _currentListName;

        void bindEvents();
    };
} // namespace app::presenters