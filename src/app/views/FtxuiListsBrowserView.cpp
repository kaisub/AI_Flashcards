#include "app/views/FtxuiListsBrowserView.hpp"
#include "app/localization/Localization.hpp"
#include "app/views/ViewTheme.hpp"
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
        } else {
            main_component = buildBrowserView(screen, return_to_controller);
        }
        screen.Loop(main_component);
    }
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
        if (event == ftxui::Event::Escape) {
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
        if (event == ftxui::Event::Escape) {
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
        if (event == ftxui::Event::Escape) {
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
        if (event == ftxui::Event::Escape) {
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

    ftxui::MenuOption menu_option;
    menu_option.on_enter = [this, &screen, &returnToController] {
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

    menu_option.entries_option.transform = [](const ftxui::EntryState& state) {
        ftxui::Element e = ftxui::text(state.label);
        if (state.label.starts_with(txt::lists_browser::kDirectoryPrefix)) {
            e = e | ftxui::color(ftxui::Color::Cyan);
        } else {
            e = e | ftxui::color(ftxui::Color::GreenLight);
        }
        if (state.active) {
            e = e | ftxui::bold;
        }
        if (state.focused) {
            e = e | ftxui::bgcolor(ftxui::Color::GrayDark) | ftxui::color(ftxui::Color::White) | ftxui::bold;
        }
        return e;
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

    auto buttons_container = ftxui::Container::Horizontal({
        back_button, exit_button, btn_new_folder, btn_new_list, btn_rename, btn_delete
    });

    auto container = ftxui::Container::Vertical({buttons_container, menu});
    if (!_menuEntries.empty()) {
        container->SetActiveChild(menu.get());
    }

    auto renderer = ftxui::Renderer(container, [this, buttons_container, menu] {
        return ftxui::vbox({
            ftxui::text(txt::lists_browser::kTitle) | ftxui::bold | ftxui::color(ftxui::Color::CyanLight) | ftxui::center,
            blueSep(),
            ftxui::text(txt::lists_browser::currentPath(_vm.currentPath.string())) | ftxui::bold | ftxui::color(ftxui::Color::White),
            blueSep(),
            buttons_container->Render(),
            blueSep(),
            menu->Render() | ftxui::vscroll_indicator | ftxui::frame | ftxui::flex,
        }) | ftxui::border | ftxui::bold | ftxui::color(ftxui::Color::BlueLight);
    });

    return ftxui::CatchEvent(renderer, [this, &screen, &returnToController](ftxui::Event event) {
        if (event.is_mouse() && event.mouse().button == ftxui::Mouse::None) {
            return true;
        }

        if (event == ftxui::Event::Escape) {
            returnToController = true;
            triggerBackRequested();
            screen.Exit();
            return true;
        }
        if (event == ftxui::Event::Character("q") || event == ftxui::Event::Character("Q")) {
            returnToController = true;
            triggerExitAppRequested();
            screen.Exit();
            return true;
        }
        if (event == ftxui::Event::Character("n") || event == ftxui::Event::Character("N")) {
            _isCreatingList = true;
            _vm.inputBuffer = "";
            screen.Exit();
            return true;
        }
        if (event == ftxui::Event::Character("f") || event == ftxui::Event::Character("F")) {
            _isCreatingFolder = true;
            _vm.inputBuffer = "";
            screen.Exit();
            return true;
        }
        if (event == ftxui::Event::Character("r") || event == ftxui::Event::Character("R")) {
            if (_vm.hasValidSelection()) {
                _isRenaming = true;
                _vm.prepareRename();
                screen.Exit();
                return true;
            }
        }
        if (event == ftxui::Event::Character("u") || event == ftxui::Event::Character("U")) {
            if (_vm.hasValidSelection()) {
                _isDeleting = true;
                screen.Exit();
                return true;
            }
        }
        return false;
    });
}

} // namespace app
