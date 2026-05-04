#pragma once
#include "app/views/IView.hpp"
#include "app/model/BrowserItem.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include <functional>

namespace app {
    class IListsBrowserView : public IView {
    public:
        using FolderOpenedHandler = std::function<void(const std::filesystem::path&)>;
        using ListSelectedHandler = std::function<void(const std::filesystem::path&)>;
        using BackRequestedHandler = std::function<void()>;
        using ExitAppRequestedHandler = std::function<void()>;
        using NewListRequestedHandler = std::function<void(const std::string& newListName)>;
        using NewFolderRequestedHandler = std::function<void(const std::string& newFolderName)>;
        using RenameRequestedHandler = std::function<void(const app::model::BrowserItem& item, const std::string& newName)>;
        using DeleteRequestedHandler = std::function<void(const app::model::BrowserItem& item)>;

        virtual ~IListsBrowserView() = default;

        // --- Controller -> View ---
        virtual void updateList(const std::filesystem::path& currentPath, 
                                const std::vector<app::model::BrowserItem>& items) = 0;

        void setOnFolderOpened(FolderOpenedHandler handler) { onFolderOpened_ = std::move(handler); }
        void setOnListSelected(ListSelectedHandler handler) { onListSelected_ = std::move(handler); }
        void setOnBackRequested(BackRequestedHandler handler) { onBackRequested_ = std::move(handler); }
        void setOnExitAppRequested(ExitAppRequestedHandler handler) { onExitAppRequested_ = std::move(handler); }
        void setOnNewListRequested(NewListRequestedHandler handler) { onNewListRequested_ = std::move(handler); }
        void setOnNewFolderRequested(NewFolderRequestedHandler handler) { onNewFolderRequested_ = std::move(handler); }
        void setOnRenameRequested(RenameRequestedHandler handler) { onRenameRequested_ = std::move(handler); }
        void setOnDeleteRequested(DeleteRequestedHandler handler) { onDeleteRequested_ = std::move(handler); }

        bool triggerFolderOpened(const std::filesystem::path& path) {
            if (!onFolderOpened_) {
                return false;
            }
            onFolderOpened_(path);
            return true;
        }

        bool triggerListSelected(const std::filesystem::path& path) {
            if (!onListSelected_) {
                return false;
            }
            onListSelected_(path);
            return true;
        }

        bool triggerBackRequested() {
            if (!onBackRequested_) {
                return false;
            }
            onBackRequested_();
            return true;
        }

        bool triggerExitAppRequested() {
            if (!onExitAppRequested_) {
                return false;
            }
            onExitAppRequested_();
            return true;
        }

        bool triggerNewListRequested(const std::string& newListName) {
            if (!onNewListRequested_) {
                return false;
            }
            onNewListRequested_(newListName);
            return true;
        }

        bool triggerNewFolderRequested(const std::string& newFolderName) {
            if (!onNewFolderRequested_) {
                return false;
            }
            onNewFolderRequested_(newFolderName);
            return true;
        }

        bool triggerRenameRequested(const app::model::BrowserItem& item, const std::string& newName) {
            if (!onRenameRequested_) {
                return false;
            }
            onRenameRequested_(item, newName);
            return true;
        }

        bool triggerDeleteRequested(const app::model::BrowserItem& item) {
            if (!onDeleteRequested_) {
                return false;
            }
            onDeleteRequested_(item);
            return true;
        }

    private:
        FolderOpenedHandler onFolderOpened_;
        ListSelectedHandler onListSelected_;
        BackRequestedHandler onBackRequested_;
        ExitAppRequestedHandler onExitAppRequested_;
        NewListRequestedHandler onNewListRequested_;
        NewFolderRequestedHandler onNewFolderRequested_;
        RenameRequestedHandler onRenameRequested_;
        DeleteRequestedHandler onDeleteRequested_;
    };
}
