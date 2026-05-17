#include "app/presenters/ListsBrowserPresenter.hpp"
#include "app/utils/AppUtils.hpp"
#include "core/FileTypeConstants.hpp"
#include <algorithm>
#include <map>

namespace app::presenters {

    ListsBrowserPresenter::ListsBrowserPresenter(std::shared_ptr<IListsBrowserView> view, core::IDeckManager* deckManager)
        : _view(std::move(view)), _deckManager(deckManager), _currentDirectory("") {
        bindEvents();
    }

    void ListsBrowserPresenter::start() {
        refreshList();
        _view->run();
    }

    void ListsBrowserPresenter::refreshList() {
        auto all_lists_paths = _deckManager->getAllAvailableLists();
        std::map<std::filesystem::path, app::model::BrowserItem> items_map;

        for (const auto& path : all_lists_paths) {
            std::error_code erc;
            auto relative_path = std::filesystem::relative(path, _currentDirectory, erc);
            if (erc || relative_path.empty() || relative_path == "." || relative_path.string().starts_with("..")) {
                continue;
            }

            auto first_component_it = relative_path.begin();
            if (first_component_it == relative_path.end()) {
                continue;
            }

            auto first_component = *first_component_it;
            auto item_path = _currentDirectory / first_component;

            bool is_dir = (++first_component_it != relative_path.end());
            if (!is_dir) {
                is_dir = (first_component.extension() != core::constants::kJsonExtension);
            }

            items_map.try_emplace(item_path, app::model::BrowserItem{first_component.string(), item_path, is_dir});
        }

        std::vector<app::model::BrowserItem> items;
        items.reserve(items_map.size());
        for (const auto& pair : items_map) {
            items.push_back(pair.second);
        }

        std::sort(items.begin(), items.end(), [](const auto& aval, const auto& bval) {
            if (aval.isDirectory != bval.isDirectory) { return aval.isDirectory > bval.isDirectory; }
            return aval.displayName < bval.displayName;
        });

        _view->updateList(_currentDirectory, items);
    }

    void ListsBrowserPresenter::bindEvents() {
        _view->setOnFolderOpened([this](const std::filesystem::path& path) {
            _currentDirectory = path;
            refreshList();
        });
        _view->setOnListSelected([this](const std::filesystem::path& path) {
            if (onListSelected) {
                onListSelected(path);
            }
        });
        
        _view->setOnBackRequested([this]() {
            if (!_currentDirectory.empty()) {
                _currentDirectory = _currentDirectory.parent_path();
                refreshList();
            } else if (onExitAppRequested) {
                onExitAppRequested();
            }
        });
        
        _view->setOnExitAppRequested([this]() {
            if (onExitAppRequested) {
                onExitAppRequested();
            }
        });

        _view->setOnNewListRequested([this](const std::string& name) {
            _deckManager->createList(name, _currentDirectory / utils::ensureJsonExtension(name));
            refreshList();
        });

        _view->setOnNewFolderRequested([this](const std::string& name) {
            _deckManager->createFolder(_currentDirectory / name);
            refreshList();
        });

        _view->setOnRenameRequested([this](const app::model::BrowserItem& item, const std::string& newName) {
            if (item.isDirectory) {
                _deckManager->renameFolder(item.fullPath, item.fullPath.parent_path() / newName);
            } else {
                _deckManager->moveListFile(item.fullPath, item.fullPath.parent_path() / utils::ensureJsonExtension(newName));
            }
            refreshList();
        });

        _view->setOnDeleteRequested([this](const app::model::BrowserItem& item) {
            if (item.isDirectory) {
                _deckManager->deleteFolder(item.fullPath);
            } else {
                _deckManager->deleteListFile(item.fullPath);
            }
            refreshList();
        });
    }
} // namespace app::presenters