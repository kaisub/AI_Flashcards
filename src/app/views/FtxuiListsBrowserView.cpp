#include "app/views/FtxuiListsBrowserView.hpp"
#include "app/localization/Localization.hpp"
#include "app/utils/BackupUtils.hpp"
#include "app/views/ViewTheme.hpp"
#include "app/views/ViewUtils.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace app {

namespace txt = ::app::localization::selected;

void FtxuiListsBrowserView::updateList(const std::filesystem::path& currentPath, const std::vector<app::model::BrowserItem>& items) {
    _vm.updateList(currentPath, items);
    
    _menuEntries.clear();
    for (const auto& item : _vm.items) {
        // Format the display string based on item type
        const std::string prefix = item.isDirectory ? txt::lists_browser::kDirectoryPrefix : txt::lists_browser::kFilePrefix;
        _menuEntries.push_back(prefix + item.displayName);
    }
}

void FtxuiListsBrowserView::run() {
    using namespace app::ui;
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    bool return_to_controller = false;
    while (!return_to_controller) {
        ftxui::Component main_component;
        if (_isCreatingList) {
            main_component = buildCreateListModal(screen, return_to_controller);
        } else if (_isCreatingFolder) {
            main_component = buildCreateFolderModal(screen, return_to_controller);
        } else if (_isRenaming) {
            main_component = buildRenameModal(screen, return_to_controller);
        } else if (_isDeleting) {
            main_component = buildDeleteModal(screen, return_to_controller);
        } else if (_isBackupPickerOpen) {
            main_component = buildBackupDirPicker(screen);
        } else if (_isBackupOverwriteConfirm) {
            main_component = buildBackupOverwriteConfirm(screen);
        } else if (_isBackupDialogOpen) {
            main_component = buildBackupDialog(screen);
        } else if (_isSettingsOpen) {
            main_component = buildSettingsModal(screen);
        } else {
            main_component = buildBrowserView(screen, return_to_controller);
        }
        screen.Loop(main_component);
    }
}

ftxui::Component FtxuiListsBrowserView::buildSettingsModal(ftxui::ScreenInteractive& screen) {
    using namespace app::ui;

    auto custom_btn_style = buttonStyle();
    auto language_button = ftxui::Button(txt::lists_browser::kLanguageButton, [this, &screen] {
        app::localization::toggleLocale();
        screen.Exit();
    }, custom_btn_style);

    auto copy_button = ftxui::Button(txt::lists_browser::kMakeCopyButton, [this, &screen] {
        _backupTargetDir = std::filesystem::current_path();
        _backupFileName = "flashcards_backup";
        _backupStatusMessage.clear();
        _backupLastSuccess = true;
        _isBackupDialogOpen = true;
        screen.Exit();
    }, custom_btn_style);

    auto restore_button = ftxui::Button(txt::lists_browser::kRestoreButton, [] {
        // Placeholder action for future implementation.
    }, custom_btn_style);

    auto exit_button = ftxui::Button(txt::common::kBackEscape, [this, &screen] {
        _isSettingsOpen = false;
        screen.Exit();
    }, custom_btn_style);

    auto settings_container = ftxui::Container::Vertical({
        language_button,
        copy_button,
        restore_button,
        exit_button
    });
    settings_container->SetActiveChild(language_button.get());

    auto settings_renderer = ftxui::Renderer(settings_container, [language_button, copy_button, restore_button, exit_button] {
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kSettingsDialogTitle) | ftxui::bold | ftxui::center,
            blueSep(),
            ftxui::text(""),
            language_button->Render(),
            ftxui::text(""),
            copy_button->Render(),
            ftxui::text(""),
            restore_button->Render(),
            ftxui::text(""),
            blueSep(),
            exit_button->Render(),
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight) | ftxui::center | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 50);
    });

    return ftxui::CatchEvent(settings_renderer, [this, &screen](ftxui::Event event) {
        if (event.is_mouse() && event.mouse().button == ftxui::Mouse::None) {
            return true;
        }

        if (app::views::utils::isEscape(event)) {
            _isSettingsOpen = false;
            screen.Exit();
            return true;
        }

        if (app::views::utils::isCharInsensitive(event, txt::common::kLanguageShortcut)) {
            app::localization::toggleLocale();
            screen.Exit();
            return true;
        }

        if (app::views::utils::isCharInsensitive(event, 'c')) {
            _backupTargetDir = std::filesystem::current_path();
            _backupFileName = "flashcards_backup";
            _backupStatusMessage.clear();
            _backupLastSuccess = true;
            _isBackupDialogOpen = true;
            screen.Exit();
            return true;
        }

        if (app::views::utils::isCharInsensitive(event, 'r')) {
            return true;
        }

        return false;
    });
}

void FtxuiListsBrowserView::refreshBackupDirPicker() {
    _backupPickerMenuEntries.clear();
    _backupPickerDirPaths.clear();

    if (_backupPickerCurrentPath.has_parent_path() &&
        _backupPickerCurrentPath != _backupPickerCurrentPath.parent_path()) {
        _backupPickerMenuEntries.emplace_back("[..] ..");
        _backupPickerDirPaths.push_back(_backupPickerCurrentPath.parent_path());
    }

    std::vector<std::filesystem::path> dirs;
    std::error_code erc;
    for (const auto& entry : std::filesystem::directory_iterator(_backupPickerCurrentPath, erc)) {
        const std::string fname = entry.path().filename().string();
        if (!fname.empty() && fname.front() != '.' && entry.is_directory(erc)) {
            dirs.push_back(entry.path());
        }
    }
    std::sort(dirs.begin(), dirs.end());
    for (const auto& dir : dirs) {
        _backupPickerMenuEntries.emplace_back("[DIR] " + dir.filename().string());
        _backupPickerDirPaths.push_back(dir);
    }
    _backupPickerSelectedIndex = 0;
}

ftxui::Component FtxuiListsBrowserView::buildBackupDialog(ftxui::ScreenInteractive& screen) {
    using namespace app::ui;

    const ftxui::InputOption input_option;
    auto name_input = ftxui::Input(&_backupFileName, txt::lists_browser::kBackupFileNamePlaceholder, input_option);

    auto browse_btn = ftxui::Button(txt::lists_browser::kBackupBrowseButton, [this, &screen] {
        _backupPickerCurrentPath = _backupTargetDir;
        refreshBackupDirPicker();
        _isBackupPickerOpen = true;
        screen.Exit();
    });

    auto do_save = [this, &screen] {
        if (_backupFileName.empty()) {
            _backupStatusMessage = txt::lists_browser::kBackupPathEmpty.str();
            _backupLastSuccess = false;
            return;
        }

        const std::filesystem::path sourceDir{"data"};
        if (!std::filesystem::exists(sourceDir) || !std::filesystem::is_directory(sourceDir)) {
            _backupStatusMessage = txt::lists_browser::kBackupSourceMissing.str();
            _backupLastSuccess = false;
            return;
        }

        _backupComputedPath = app::utils::buildBackupOutputPath(_backupTargetDir, _backupFileName);

        if (std::filesystem::exists(_backupComputedPath)) {
            _isBackupOverwriteConfirm = true;
            screen.Exit();
            return;
        }

        const bool ok = app::utils::createZipBackup(_backupComputedPath);
        _backupLastSuccess = ok;
        _backupStatusMessage = ok ? txt::lists_browser::kBackupSaved.str()
                                  : txt::lists_browser::kBackupFailed.str();
        if (ok) {
            _isBackupDialogOpen = false;
            screen.Exit();
        }
    };

    auto save_btn = ftxui::Button(txt::lists_browser::kBackupSaveButton, do_save);
    auto close_btn = ftxui::Button(txt::common::kBackEscape, [this, &screen] {
        _isBackupDialogOpen = false;
        screen.Exit();
    });

    auto container = ftxui::Container::Vertical({
        browse_btn,
        name_input,
        ftxui::Container::Horizontal({save_btn, close_btn})
    });
    container->SetActiveChild(name_input.get());

    auto renderer = ftxui::Renderer(container, [this, browse_btn, name_input, save_btn, close_btn] {
        const ftxui::Color status_color = _backupLastSuccess ? ftxui::Color::GreenLight : ftxui::Color::Red;
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kBackupDialogTitle) | ftxui::bold | ftxui::center,
            blueSep(),
            ftxui::text(txt::lists_browser::kBackupTargetDirLabel) | ftxui::bold,
            ftxui::text(" " + _backupTargetDir.string() + " ") | ftxui::color(ftxui::Color::White),
            browse_btn->Render() | ftxui::center,
            ftxui::text(""),
            ftxui::text(txt::lists_browser::kBackupFileNameLabel) | ftxui::bold,
            name_input->Render(),
            ftxui::text(""),
            ftxui::hbox({
                save_btn->Render() | ftxui::flex,
                ftxui::text("  "),
                close_btn->Render() | ftxui::flex
            }),
            ftxui::text(""),
            ftxui::text(_backupStatusMessage) | ftxui::color(status_color),
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight) | ftxui::center | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 70);
    });

    return ftxui::CatchEvent(renderer, [this, &screen](ftxui::Event event) {
        if (event.is_mouse() && event.mouse().button == ftxui::Mouse::None) {
            return true;
        }

        if (app::views::utils::isEscape(event)) {
            _isBackupDialogOpen = false;
            screen.Exit();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiListsBrowserView::buildBackupDirPicker(ftxui::ScreenInteractive& screen) {
    using namespace app::ui;

    auto activate_selected = [this] {
        if (_backupPickerSelectedIndex >= 0 &&
            _backupPickerSelectedIndex < static_cast<int>(_backupPickerDirPaths.size())) {
            _backupPickerCurrentPath = _backupPickerDirPaths[_backupPickerSelectedIndex];
            refreshBackupDirPicker();
        }
    };

    ftxui::MenuOption menu_opt;

    auto menu = ftxui::Menu(&_backupPickerMenuEntries, &_backupPickerSelectedIndex, menu_opt);

    auto select_btn = ftxui::Button(txt::lists_browser::kBackupSelectFolderButton, [this, &screen] {
        _backupTargetDir = _backupPickerCurrentPath;
        _isBackupPickerOpen = false;
        screen.Exit();
    });

    auto cancel_btn = ftxui::Button(txt::common::kCancelEscape, [this, &screen] {
        _isBackupPickerOpen = false;
        screen.Exit();
    });

    auto container = ftxui::Container::Vertical({
        menu,
        ftxui::Container::Horizontal({select_btn, cancel_btn})
    });
    container->SetActiveChild(menu.get());
    auto menu_box = std::make_shared<ftxui::Box>();
    auto click_armed = std::make_shared<bool>(false);
    auto last_clicked_index = std::make_shared<int>(-1);

    auto renderer = ftxui::Renderer(container, [this, menu, select_btn, cancel_btn, menu_box] {
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kBackupPickerTitle) | ftxui::bold | ftxui::color(ftxui::Color::CyanLight) | ftxui::center,
            blueSep(),
            ftxui::text(" " + _backupPickerCurrentPath.string() + " ") | ftxui::color(ftxui::Color::White) | ftxui::center,
            blueSep(),
            menu->Render() | ftxui::vscroll_indicator | ftxui::frame | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 15) | ftxui::reflect(*menu_box),
            blueSep(),
            ftxui::hbox({
                select_btn->Render() | ftxui::flex,
                ftxui::text("  "),
                cancel_btn->Render() | ftxui::flex
            })
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight) | ftxui::center | ftxui::size(ftxui::WIDTH, ftxui::LESS_THAN, 80);
    });

    return ftxui::CatchEvent(renderer, [this, &screen, menu, menu_box, click_armed, last_clicked_index, activate_selected](ftxui::Event event) {
        if (app::views::utils::handleMenuTwoClickMouseSelect(
                event,
                menu,
                menu_box,
                _backupPickerSelectedIndex,
                _backupPickerMenuEntries.size(),
                *click_armed,
                *last_clicked_index,
                activate_selected)) {
            return true;
        }

        if (event == ftxui::Event::Return && menu->Focused()) {
            activate_selected();
            return true;
        }

        if (app::views::utils::isEscape(event)) {
            _isBackupPickerOpen = false;
            screen.Exit();
            return true;
        }
        if (app::views::utils::isCharInsensitive(event, 's')) {
            _backupTargetDir = _backupPickerCurrentPath;
            _isBackupPickerOpen = false;
            screen.Exit();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiListsBrowserView::buildBackupOverwriteConfirm(ftxui::ScreenInteractive& screen) {
    using namespace app::ui;

    auto yes_btn = ftxui::Button(txt::common::kYesEnter, [this, &screen] {
        const bool ok = app::utils::createZipBackup(_backupComputedPath);
        _backupLastSuccess = ok;
        _backupStatusMessage = ok ? txt::lists_browser::kBackupSaved.str()
                                  : txt::lists_browser::kBackupFailed.str();
        _isBackupOverwriteConfirm = false;
        if (ok) {
            _isBackupDialogOpen = false;
        }
        screen.Exit();
    });

    auto no_btn = ftxui::Button(txt::common::kNoEscape, [this, &screen] {
        _isBackupOverwriteConfirm = false;
        screen.Exit();
    });

    auto container = ftxui::Container::Horizontal({yes_btn, no_btn});
    container->SetActiveChild(no_btn.get());

    auto renderer = ftxui::Renderer(container, [this, yes_btn, no_btn] {
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kBackupOverwriteQuestion) | ftxui::bold | ftxui::center,
            ftxui::text(_backupComputedPath.string()) | ftxui::color(ftxui::Color::White) | ftxui::center,
            blueSep(),
            ftxui::text(""),
            ftxui::hbox({
                yes_btn->Render() | ftxui::flex,
                ftxui::text("  "),
                no_btn->Render() | ftxui::flex
            }) | ftxui::center
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight) | ftxui::center | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 60);
    });

    return ftxui::CatchEvent(renderer, [this, &screen](const ftxui::Event& event) {
        if (app::views::utils::isEscape(event)) {
            _isBackupOverwriteConfirm = false;
            screen.Exit();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiListsBrowserView::buildCreateListModal(ftxui::ScreenInteractive& screen, bool& returnToController) {
    using namespace app::ui;
    const ftxui::InputOption input_option;
    auto create_action = [this, &screen, &returnToController] {
        if (!_vm.inputBuffer.empty()) {
            _vm.pendingSelection = _vm.inputBuffer;
            triggerNewListRequested(_vm.inputBuffer);
        }
        _isCreatingList = false;
        returnToController = true;
        screen.Exit();
    };
    auto input_component = ftxui::Input(&_vm.inputBuffer, txt::lists_browser::kListNamePlaceholder, input_option);
    auto create_button = ftxui::Button(txt::common::kCreateEnter, create_action);
    auto cancel_button = ftxui::Button(txt::common::kCancelEscape, [this, &screen] {
        _isCreatingList = false;
        screen.Exit();
    });

    auto create_container = ftxui::Container::Vertical({
        input_component,
        ftxui::Container::Horizontal({create_button, cancel_button})
    });
    create_container->SetActiveChild(input_component.get());

    auto create_renderer = ftxui::Renderer(create_container, [input_component, create_button, cancel_button] {
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kCreateListPrompt) | ftxui::bold | ftxui::center,
            blueSep(),
            ftxui::text(""),
            input_component->Render(),
            ftxui::text(""),
            blueSep(),
            ftxui::hbox({
                create_button->Render() | ftxui::flex,
                ftxui::text("  "),
                cancel_button->Render() | ftxui::flex
            })
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight) | ftxui::center | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 50);
    });

    return ftxui::CatchEvent(create_renderer, [this, &screen, create_action](const ftxui::Event& event) {
        if (app::views::utils::isEscape(event)) {
            _isCreatingList = false;
            screen.Exit();
            return true;
        }
        if (event == ftxui::Event::Return) {
            create_action();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiListsBrowserView::buildCreateFolderModal(ftxui::ScreenInteractive& screen, bool& returnToController) {
    using namespace app::ui;
    const ftxui::InputOption input_option;
    auto create_action = [this, &screen, &returnToController] {
        if (!_vm.inputBuffer.empty()) {
            _vm.pendingSelection = _vm.inputBuffer;
            triggerNewFolderRequested(_vm.inputBuffer);
        }
        _isCreatingFolder = false;
        returnToController = true;
        screen.Exit();
    };
    auto input_component = ftxui::Input(&_vm.inputBuffer, txt::lists_browser::kFolderNamePlaceholder, input_option);
    auto create_button = ftxui::Button(txt::common::kCreateEnter, create_action);
    auto cancel_button = ftxui::Button(txt::common::kCancelEscape, [this, &screen] {
        _isCreatingFolder = false;
        screen.Exit();
    });

    auto create_container = ftxui::Container::Vertical({
        input_component,
        ftxui::Container::Horizontal({create_button, cancel_button})
    });
    create_container->SetActiveChild(input_component.get());

    auto create_renderer = ftxui::Renderer(create_container, [input_component, create_button, cancel_button] {
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kCreateFolderPrompt) | ftxui::bold | ftxui::center,
            blueSep(),
            ftxui::text(""),
            input_component->Render(),
            ftxui::text(""),
            blueSep(),
            ftxui::hbox({
                create_button->Render() | ftxui::flex,
                ftxui::text("  "),
                cancel_button->Render() | ftxui::flex
            })
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight) | ftxui::center | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 50);
    });

    return ftxui::CatchEvent(create_renderer, [this, &screen, create_action](const ftxui::Event& event) {
        if (app::views::utils::isEscape(event)) {
            _isCreatingFolder = false;
            screen.Exit();
            return true;
        }
        if (event == ftxui::Event::Return) {
            create_action();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiListsBrowserView::buildRenameModal(ftxui::ScreenInteractive& screen, bool& returnToController) {
    using namespace app::ui;
    ftxui::InputOption input_option;
    input_option.cursor_position = static_cast<int>(_vm.inputBuffer.size());
    auto rename_action = [this, &screen, &returnToController] {
        if (!_vm.inputBuffer.empty() && _vm.hasValidSelection()) {
            _vm.pendingSelection = _vm.inputBuffer;
            triggerRenameRequested(_vm.getSelectedItem(), _vm.inputBuffer);
        }
        _isRenaming = false;
        returnToController = true;
        screen.Exit();
    };
    auto input_component = ftxui::Input(&_vm.inputBuffer, txt::lists_browser::kRenamePlaceholder, input_option);
    auto rename_button = ftxui::Button(txt::lists_browser::kRenameButton, rename_action);
    auto cancel_button = ftxui::Button(txt::common::kCancelEscape, [this, &screen] {
        _isRenaming = false;
        screen.Exit();
    });

    auto rename_container = ftxui::Container::Vertical({
        input_component,
        ftxui::Container::Horizontal({rename_button, cancel_button})
    });
    rename_container->SetActiveChild(input_component.get());

    auto rename_renderer = ftxui::Renderer(rename_container, [input_component, rename_button, cancel_button] {
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kRenamePrompt) | ftxui::bold | ftxui::center,
            blueSep(),
            ftxui::text(""),
            input_component->Render(),
            ftxui::text(""),
            blueSep(),
            ftxui::hbox({
                rename_button->Render() | ftxui::flex,
                ftxui::text("  "),
                cancel_button->Render() | ftxui::flex
            })
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight) | ftxui::center | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 50);
    });

    return ftxui::CatchEvent(rename_renderer, [this, &screen, rename_action](const ftxui::Event& event) {
        if (app::views::utils::isEscape(event)) {
            _isRenaming = false;
            screen.Exit();
            return true;
        }
        if (event == ftxui::Event::Return) {
            rename_action();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiListsBrowserView::buildDeleteModal(ftxui::ScreenInteractive& screen, bool& returnToController) {
    using namespace app::ui;

    auto yes_button = ftxui::Button(txt::common::kYesEnter, [this, &screen, &returnToController] {
        if (_vm.hasValidSelection()) {
            triggerDeleteRequested(_vm.getSelectedItem());
        }
        _isDeleting = false;
        returnToController = true;
        screen.Exit();
    });

    auto no_button = ftxui::Button(txt::common::kNoEscape, [this, &screen] {
        _isDeleting = false;
        screen.Exit();
    });

    auto delete_container = ftxui::Container::Horizontal({yes_button, no_button});
    delete_container->SetActiveChild(yes_button.get());

    auto delete_renderer = ftxui::Renderer(delete_container, [this, yes_button, no_button] {
        const std::string item_name = _vm.hasValidSelection()
            ? _vm.getSelectedItem().displayName
            : txt::common::kQuestionMark;
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kDeletePrompt) | ftxui::bold | ftxui::center,
            ftxui::text(item_name) | ftxui::bold | ftxui::center,
            blueSep(),
            ftxui::text(""),
            ftxui::hbox({
                yes_button->Render() | ftxui::flex,
                ftxui::text("  "),
                no_button->Render() | ftxui::flex
            }) | ftxui::center
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight) | ftxui::center | ftxui::size(ftxui::WIDTH, ftxui::GREATER_THAN, 50);
    });

    return ftxui::CatchEvent(delete_renderer, [this, &screen](const ftxui::Event& event) {
        if (app::views::utils::isEscape(event)) {
            _isDeleting = false;
            screen.Exit();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiListsBrowserView::buildBrowserView(ftxui::ScreenInteractive& screen, bool& returnToController) {
    using namespace app::ui;

    const int target_index = std::max(0, _vm.selectedIndex);
    _vm.selectedIndex = 0;
    const std::string directory_prefix = txt::lists_browser::kDirectoryPrefix.str();

    auto activate_selected = [this, &screen, &returnToController] {
        if (!_vm.hasValidSelection()) {
            return;
        }
        const auto selected = _vm.getSelectedItem();
        if (selected.isDirectory) {
            triggerFolderOpened(selected.fullPath);
        } else {
            triggerListSelected(selected.fullPath);
        }
        returnToController = true;
        screen.Exit();
    };

    ftxui::MenuOption menu_option;

    menu_option.entries_option.transform = [directory_prefix](const ftxui::EntryState& state) {
        ftxui::Element elm = ftxui::text(state.label);
        if (state.label.starts_with(directory_prefix)) {
            elm = elm | ftxui::color(ftxui::Color::Cyan);
        } else {
            elm = elm | ftxui::color(ftxui::Color::GreenLight);
        }
        if (state.active) {
            elm = elm | ftxui::bold;
        }
        if (state.focused) {
            elm = elm | ftxui::bgcolor(ftxui::Color::GrayDark) | ftxui::color(ftxui::Color::White) | ftxui::bold;
        }
        return elm;
    };

    auto menu = ftxui::Menu(&_menuEntries, &_vm.selectedIndex, menu_option);
    for (int i = 0; i < target_index; ++i) {
        menu->OnEvent(ftxui::Event::ArrowDown);
    }

    auto custom_btn_style = buttonStyle();

    auto back_button = ftxui::Button(txt::common::kBackEscape, [this, &screen, &returnToController] {
        returnToController = true;
        triggerBackRequested();
        screen.Exit();
    }, custom_btn_style);

    auto exit_button = ftxui::Button(txt::common::kExitQ, [this, &screen, &returnToController] {
        returnToController = true;
        triggerExitAppRequested();
        screen.Exit();
    }, custom_btn_style);

    auto btn_new_folder = ftxui::Button(txt::lists_browser::kNewFolderButton, [this, &screen] {
        _isCreatingFolder = true;
        _vm.inputBuffer = "";
        screen.Exit();
    }, custom_btn_style);

    auto btn_new_list = ftxui::Button(txt::lists_browser::kNewListButton, [this, &screen] {
        _isCreatingList = true;
        _vm.inputBuffer = "";
        screen.Exit();
    }, custom_btn_style);

    auto btn_rename = ftxui::Button(txt::lists_browser::kRenameToolbarButton, [this, &screen] {
        if (_vm.hasValidSelection()) {
            _isRenaming = true;
            _vm.prepareRename();
            screen.Exit();
        }
    }, custom_btn_style);

    auto btn_delete = ftxui::Button(txt::lists_browser::kDeleteToolbarButton, [this, &screen] {
        if (_vm.hasValidSelection()) {
            _isDeleting = true;
            screen.Exit();
        }
    }, custom_btn_style);

    auto btn_settings = ftxui::Button(txt::lists_browser::kSettingsButton, [this, &screen] {
        _isSettingsOpen = true;
        screen.Exit();
    }, custom_btn_style);

    auto buttons_container = ftxui::Container::Horizontal({
        back_button, exit_button, btn_new_folder, btn_new_list, btn_rename, btn_delete, btn_settings
    });

    auto container = ftxui::Container::Vertical({buttons_container, menu});
    if (!_menuEntries.empty()) {
        container->SetActiveChild(menu.get());
    }

    auto menu_box = std::make_shared<ftxui::Box>();
    auto click_armed = std::make_shared<bool>(false);
    auto last_clicked_index = std::make_shared<int>(-1);

    auto renderer = ftxui::Renderer(container, [this, buttons_container, menu, menu_box] {
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kTitle) | ftxui::bold | ftxui::color(ftxui::Color::CyanLight) | ftxui::center,
            blueSep(),
            ftxui::text(txt::lists_browser::currentPath(_vm.currentPath.string())) | ftxui::bold | ftxui::color(ftxui::Color::White),
            blueSep(),
            buttons_container->Render(),
            blueSep(),
            menu->Render() | ftxui::vscroll_indicator | ftxui::frame | ftxui::flex | ftxui::reflect(*menu_box),
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight);
    });

    return ftxui::CatchEvent(renderer, [this, &screen, &returnToController, menu, menu_box, click_armed, last_clicked_index, activate_selected](ftxui::Event event) {
        if (app::views::utils::handleMenuTwoClickMouseSelect(
                event,
                menu,
                menu_box,
                _vm.selectedIndex,
                _menuEntries.size(),
                *click_armed,
                *last_clicked_index,
                activate_selected)) {
            return true;
        }

        if (event == ftxui::Event::Return && menu->Focused()) {
            activate_selected();
            return true;
        }

        if (app::views::utils::isEscape(event)) {
            returnToController = true;
            triggerBackRequested();
            screen.Exit();
            return true;
        }
        if (app::views::utils::isCharInsensitive(event, txt::common::kExitApp)) {
            returnToController = true;
            triggerExitAppRequested();
            screen.Exit();
            return true;
        }
        if (app::views::utils::isCharInsensitive(event, txt::lists_browser::kNewListShortcut)) {
            _isCreatingList = true;
            _vm.inputBuffer = "";
            screen.Exit();
            return true;
        }
        if (app::views::utils::isCharInsensitive(event, txt::lists_browser::kNewFolderShortcut)) {
            _isCreatingFolder = true;
            _vm.inputBuffer = "";
            screen.Exit();
            return true;
        }
        if (app::views::utils::isCharInsensitive(event, txt::lists_browser::kRenameShortcut)) {
            if (_vm.hasValidSelection()) {
                _isRenaming = true;
                _vm.prepareRename();
                screen.Exit();
                return true;
            }
        }
        if (app::views::utils::isCharInsensitive(event, txt::lists_browser::kDeleteShortcut)) {
            if (_vm.hasValidSelection()) {
                _isDeleting = true;
                screen.Exit();
                return true;
            }
        }
        if (app::views::utils::isCharInsensitive(event, txt::lists_browser::kSettingsShortcut)) {
            _isSettingsOpen = true;
            screen.Exit();
            return true;
        }
        return false;
    });
}

} // namespace app
