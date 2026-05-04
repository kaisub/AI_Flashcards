#pragma once

#include "app/views/IListsBrowserView.hpp"
#include "core/IDeckManager.hpp"
#include <filesystem>
#include <memory>
#include <functional>

namespace app::presenters {

    class ListsBrowserPresenter {
    public:
        ListsBrowserPresenter(std::shared_ptr<IListsBrowserView> view, core::IDeckManager* deckManager);

        void start();

        // Callbacks for AppController transitions
        std::function<void(const std::filesystem::path&)> onListSelected;
        std::function<void()> onExitAppRequested;

    private:
        std::shared_ptr<IListsBrowserView> _view;
        core::IDeckManager* _deckManager;
        std::filesystem::path _currentDirectory;

        void bindEvents();
        void refreshList();
    };
} // namespace app::presenters