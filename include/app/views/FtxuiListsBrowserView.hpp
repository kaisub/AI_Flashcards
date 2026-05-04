#pragma once

#include "app/views/IListsBrowserView.hpp"
#include "app/model/ListsBrowserViewModel.hpp"
#include "app/model/BrowserItem.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <filesystem>
#include <vector>
#include <string>

namespace app {

class FtxuiListsBrowserView : public IListsBrowserView {
public:
    FtxuiListsBrowserView() = default;
    ~FtxuiListsBrowserView() override = default;

    // IListsBrowserView overrides
    void updateList(const std::filesystem::path& currentPath, const std::vector<app::model::BrowserItem>& items) override;
    void run() override;

private:
    app::model::ListsBrowserViewModel _vm;
    
    // FTXUI Specific State
    std::vector<std::string> _menuEntries;
    
    // CRUD modal states
    bool _isCreatingList = false;
    bool _isCreatingFolder = false;
    bool _isRenaming = false;
    bool _isDeleting = false;

    // Modal / view builders
    ftxui::Component buildCreateListModal(ftxui::ScreenInteractive& screen, bool& returnToController);
    ftxui::Component buildCreateFolderModal(ftxui::ScreenInteractive& screen, bool& returnToController);
    ftxui::Component buildRenameModal(ftxui::ScreenInteractive& screen, bool& returnToController);
    ftxui::Component buildDeleteModal(ftxui::ScreenInteractive& screen, bool& returnToController);
    ftxui::Component buildBrowserView(ftxui::ScreenInteractive& screen, bool& returnToController);
};

} // namespace app
