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
    bool _isSettingsOpen = false;
    bool _isBackupDialogOpen = false;
    bool _isBackupPickerOpen = false;
    bool _isBackupOverwriteConfirm = false;
    bool _isRestorePickerOpen = false;
    bool _backupLastSuccess = true;
    bool _restoreLastSuccess = true;

    std::filesystem::path _backupTargetDir;
    std::string _backupFileName;
    std::string _backupStatusMessage;
    std::filesystem::path _backupComputedPath;

    std::filesystem::path _backupPickerCurrentPath;
    std::vector<std::string> _backupPickerMenuEntries;
    std::vector<std::filesystem::path> _backupPickerDirPaths;
    int _backupPickerSelectedIndex = 0;

    std::string _restoreStatusMessage;
    std::filesystem::path _restorePickerCurrentPath;
    std::vector<std::string> _restorePickerMenuEntries;
    std::vector<std::filesystem::path> _restorePickerPaths;
    int _restorePickerSelectedIndex = 0;

    // Modal / view builders
    ftxui::Component buildCreateListModal(ftxui::ScreenInteractive& screen, bool& returnToController);
    ftxui::Component buildCreateFolderModal(ftxui::ScreenInteractive& screen, bool& returnToController);
    ftxui::Component buildRenameModal(ftxui::ScreenInteractive& screen, bool& returnToController);
    ftxui::Component buildDeleteModal(ftxui::ScreenInteractive& screen, bool& returnToController);
    ftxui::Component buildSettingsModal(ftxui::ScreenInteractive& screen);
    ftxui::Component buildBackupDialog(ftxui::ScreenInteractive& screen);
    ftxui::Component buildBackupDirPicker(ftxui::ScreenInteractive& screen);
    ftxui::Component buildBackupOverwriteConfirm(ftxui::ScreenInteractive& screen);
    ftxui::Component buildRestoreFilePicker(ftxui::ScreenInteractive& screen);
    void refreshBackupDirPicker();
    void refreshRestoreFilePicker();
    ftxui::Component buildBrowserView(ftxui::ScreenInteractive& screen, bool& returnToController);
};

} // namespace app
