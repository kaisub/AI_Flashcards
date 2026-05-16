#include "app/views/FtxuiDeckEditorView.hpp"
#include "app/localization/Localization.hpp"
#include "app/views/ViewTheme.hpp"
#include <algorithm>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>

namespace app {

namespace txt = ::app::localization::selected;

void FtxuiDeckEditorView::setDeck(std::shared_ptr<core::FlashcardList> deck) {
    _vm.setDeck(std::move(deck));
    if (_onDeckChangedInternal) {
        _onDeckChangedInternal();
    }
}

void FtxuiDeckEditorView::setAvailableLists(const std::vector<std::string>& listNames) {
    _vm.setAvailableLists(listNames);
}

void FtxuiDeckEditorView::refreshFilePicker() {
    if (_pickerCurrentPath.empty()) {
        _pickerCurrentPath = std::filesystem::current_path();
    }
    _pickerMenuEntries.clear();
    _pickerFullPaths.clear();

    if (_pickerCurrentPath.has_parent_path() && _pickerCurrentPath != _pickerCurrentPath.parent_path()) {
        _pickerMenuEntries.emplace_back("[DIR] ..");
        _pickerFullPaths.push_back(_pickerCurrentPath.parent_path());
    }

    std::vector<std::filesystem::path> dirs;
    std::vector<std::filesystem::path> files;

    std::error_code erc;
    for (const auto& entry : std::filesystem::directory_iterator(_pickerCurrentPath, erc)) {
        std::string filename = entry.path().filename().string();
        if (filename.empty() || filename.front() == '.') {
            continue; // Skip hidden items
        }
        if (entry.is_directory(erc)) {
            dirs.push_back(entry.path());
        } else if (entry.is_regular_file(erc)) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".csv" || ext == ".txt" || ext == ".tsv") {
                files.push_back(entry.path());
            }
        }
    }

    std::sort(dirs.begin(), dirs.end());
    std::sort(files.begin(), files.end());

    for (const auto& dir : dirs) {
        _pickerMenuEntries.emplace_back("[DIR] " + dir.filename().string());
        _pickerFullPaths.push_back(dir);
    }
    for (const auto& file : files) {
        _pickerMenuEntries.emplace_back("[FILE] " + file.filename().string());
        _pickerFullPaths.push_back(file);
    }
    _pickerSelectedIndex = 0;
}

ftxui::Component FtxuiDeckEditorView::buildFilePickerModal(const ftxui::ButtonOption& btnStyle) {
    using namespace ftxui;
    using namespace app::ui;

    ftxui::MenuOption menu_opt;
    menu_opt.on_enter = [this] {
        if (_pickerSelectedIndex >= 0 && _pickerSelectedIndex < static_cast<int>(_pickerFullPaths.size())) {
            auto selected = _pickerFullPaths[_pickerSelectedIndex];
            std::error_code erc;
            if (std::filesystem::is_directory(selected, erc)) {
                _pickerCurrentPath = selected;
                refreshFilePicker();
            } else {
                _importPath = selected.string();
                _isFilePickerActive = false;
            }
        }
    };

    auto menu = Menu(&_pickerMenuEntries, &_pickerSelectedIndex, menu_opt);
    auto cancel_btn = Button(txt::common::kCancelEscape, [this]{ _isFilePickerActive = false; }, btnStyle);

    auto container = Container::Vertical({
        menu,
        cancel_btn
    });

    auto renderer = Renderer(container, [this, menu, cancel_btn] {
        return vbox({
            text(txt::deck_editor::kFilePickerTitle) | bold | color(Color::CyanLight) | center,
            blueSep(),
            text(" " + _pickerCurrentPath.string() + " ") | color(Color::White) | center,
            blueSep(),
            menu->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 15),
            blueSep(),
            cancel_btn->Render() | center
        }) | border | bold | color(Color::BlueLight) | center | size(WIDTH, LESS_THAN, 80);
    });

    return CatchEvent(renderer, [this](const Event& event) {
        if (event == Event::Escape) {
            _isFilePickerActive = false;
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiDeckEditorView::buildEditModal(const ftxui::ButtonOption& btnStyle) {
    using namespace ftxui;
    using namespace app::ui;

    ftxui::InputOption edit_front_opt; // NOLINT(misc-const-correctness)
    edit_front_opt.cursor_position = static_cast<int>(_vm.editFront.size());
    auto edit_front_input = Input(&_vm.editFront, txt::common::kFront, edit_front_opt);

    ftxui::InputOption edit_back_opt; // NOLINT(misc-const-correctness)
    edit_back_opt.cursor_position = static_cast<int>(_vm.editBack.size());
    auto edit_back_input = Input(&_vm.editBack, txt::common::kBack, edit_back_opt);

    auto edit_save_btn = Button(txt::common::kSaveEnter, [this] {
        if (!_vm.editFront.empty() && !_vm.editBack.empty()) {
            triggerUpdateCard(_vm.editCardId, _vm.editFront, _vm.editBack);
            _isEditing = false;
        }
    }, btnStyle);

    auto edit_cancel_btn = Button(txt::common::kCancelEscape, [this] {
        _isEditing = false;
    }, btnStyle);

    auto edit_container = Container::Vertical({
        edit_front_input,
        edit_back_input,
        Container::Horizontal({edit_save_btn, edit_cancel_btn}),
    });
    edit_container->SetActiveChild(edit_front_input.get());

    auto edit_renderer = Renderer(edit_container, [edit_front_input, edit_back_input, edit_save_btn, edit_cancel_btn] {
        return vbox({
            text(txt::deck_editor::kEditTitle) | bold | center,
            blueSep(),
            hbox({text(txt::common::kFrontLabel), edit_front_input->Render() | flex}),
            hbox({text(txt::common::kBackLabel), edit_back_input->Render() | flex}),
            blueSep(),
            hbox({edit_save_btn->Render(), text(" "), edit_cancel_btn->Render()}) | center
        }) | border | bold | color(Color::BlueLight) | center | size(WIDTH, LESS_THAN, 80);
    });

    return CatchEvent(edit_renderer, [this](const Event& event) {
        if (event == Event::Escape) {
            _isEditing = false;
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiDeckEditorView::buildDeleteModal(const ftxui::ButtonOption& btnStyle) {
    using namespace ftxui;
    using namespace app::ui;

    auto delete_confirm_btn = Button(txt::deck_editor::kDeleteConfirmButton, [this] {
        std::vector<std::string> ids = _vm.getSelectedCardIds();
        // If no cards are selected, use the focused card if available
        if (ids.empty() && !_focusedCardId.empty()) {
            ids.push_back(_focusedCardId);
        }
        if (!ids.empty() && triggerDeleteSelected(ids)) {
            _vm.clearSelectionFor(ids);
            _focusedCardId = "";
            _isDeletingBulk = false;
        }
    }, btnStyle);

    auto delete_cancel_btn = Button(txt::common::kCancelEscape, [this] {
        _isDeletingBulk = false;
    }, btnStyle);

    auto delete_container = Container::Vertical({
        Container::Horizontal({delete_confirm_btn, delete_cancel_btn}),
    });

    auto delete_renderer = Renderer(delete_container, [this, delete_confirm_btn, delete_cancel_btn] {
        size_t selected_count = _vm.getSelectedCount();
        // Include focused card if no cards are selected
        if (selected_count == 0 && !_focusedCardId.empty()) {
            selected_count = 1;
        }
        return vbox({
            text(txt::deck_editor::kDeleteTitle) | bold | center,
            blueSep(),
            text(txt::deck_editor::deletePrompt(selected_count)) | center,
            blueSep(),
            hbox({delete_confirm_btn->Render(), text(" "), delete_cancel_btn->Render()}) | center
        }) | border | bold | color(Color::BlueLight) | center | size(WIDTH, LESS_THAN, 60);
    });

    return CatchEvent(delete_renderer, [this](const Event& event) {
        if (event == Event::Escape) {
            _isDeletingBulk = false;
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiDeckEditorView::buildMoveModal(const ftxui::ButtonOption& btnStyle) {
    using namespace ftxui;
    using namespace app::ui;

    auto move_list_menu = Menu(&_vm.availableLists, &_vm.selectedListIndex);

    auto do_move = [this] {
        if (_vm.selectedListIndex >= 0 && _vm.selectedListIndex < static_cast<int>(_vm.availableLists.size())) {
            std::vector<std::string> ids = _vm.getSelectedCardIds();
            // If no cards are selected, use the focused card if available
            if (ids.empty() && !_focusedCardId.empty()) {
                ids.push_back(_focusedCardId);
            }
            if (!ids.empty() && triggerMoveSelected(ids, _vm.selectedListIndex)) {
                _vm.clearSelectionFor(ids);
                _focusedCardId = "";
                _isMovingBulk = false;
            }
        }
    };

    auto move_confirm_btn = Button(txt::deck_editor::kMoveConfirmButton, [do_move] { do_move(); }, btnStyle);
    auto move_cancel_btn = Button(txt::common::kCancelEscape, [this] { _isMovingBulk = false; }, btnStyle);

    auto move_container = Container::Vertical({
        move_list_menu,
        Container::Horizontal({move_confirm_btn, move_cancel_btn}),
    });

    auto move_renderer = Renderer(move_container, [this, move_list_menu, move_confirm_btn, move_cancel_btn] {
        size_t selected_count = _vm.getSelectedCount();
        // Include focused card if no cards are selected
        if (selected_count == 0 && !_focusedCardId.empty()) {
            selected_count = 1;
        }
        return vbox({
            text(txt::deck_editor::kMoveTitle) | bold | center,
            blueSep(),
            text(txt::deck_editor::movePrompt(selected_count)) | center,
            blueSep(),
            move_list_menu->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
            blueSep(),
            hbox({move_confirm_btn->Render(), text(" "), move_cancel_btn->Render()}) | center
        }) | border | bold | color(Color::BlueLight) | center | size(WIDTH, LESS_THAN, 60);
    });

    return CatchEvent(move_renderer, [this, do_move](const Event& event) {
        if (event == Event::Escape) {
            _isMovingBulk = false;
            return true;
        }
        if (event == Event::Return) {
            do_move();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiDeckEditorView::buildCopyModal(const ftxui::ButtonOption& btnStyle) {
    using namespace ftxui;
    using namespace app::ui;

    auto copy_list_menu = Menu(&_vm.availableLists, &_vm.selectedListIndex);

    auto do_copy = [this] {
        if (_vm.selectedListIndex >= 0 && _vm.selectedListIndex < static_cast<int>(_vm.availableLists.size())) {
            std::vector<std::string> ids = _vm.getSelectedCardIds();
            // If no cards are selected, use the focused card if available
            if (ids.empty() && !_focusedCardId.empty()) {
                ids.push_back(_focusedCardId);
            }
            if (!ids.empty() && triggerCopySelected(ids, _vm.selectedListIndex)) {
                _vm.clearSelectionFor(ids);
                _focusedCardId = "";
                _isCopyingBulk = false;
            }
        }
    };

    auto copy_confirm_btn = Button(txt::common::kCopyEnter, [do_copy] { do_copy(); }, btnStyle);
    auto copy_cancel_btn = Button(txt::common::kCancelEscape, [this] { _isCopyingBulk = false; }, btnStyle);

    auto copy_container = Container::Vertical({
        copy_list_menu,
        Container::Horizontal({copy_confirm_btn, copy_cancel_btn}),
    });

    auto copy_renderer = Renderer(copy_container, [this, copy_list_menu, copy_confirm_btn, copy_cancel_btn] {
        size_t selected_count = _vm.getSelectedCount();
        // Include focused card if no cards are selected
        if (selected_count == 0 && !_focusedCardId.empty()) {
            selected_count = 1;
        }
        return vbox({
            text(txt::deck_editor::kCopyTitle) | bold | center,
            blueSep(),
            text(txt::deck_editor::copyPrompt(selected_count)) | center,
            blueSep(),
            copy_list_menu->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
            blueSep(),
            hbox({copy_confirm_btn->Render(), text(" "), copy_cancel_btn->Render()}) | center
        }) | border | bold | color(Color::BlueLight) | center | size(WIDTH, LESS_THAN, 60);
    });

    return CatchEvent(copy_renderer, [this, do_copy](const Event& event) {
        if (event == Event::Escape) {
            _isCopyingBulk = false;
            return true;
        }
        if (event == Event::Return) {
            do_copy();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiDeckEditorView::buildImportModal(const ftxui::ButtonOption& btnStyle, const ftxui::ButtonOption& inlineBtnStyle) {
    using namespace ftxui;
    using namespace app::ui;

    auto select_file_btn = Button(txt::deck_editor::kSelectFileButton, [this]{ 
       _isFilePickerActive = true; 
       refreshFilePicker(); 
   }, inlineBtnStyle);

    auto delimiter_radiobox = Radiobox(&_importDelimiterOptions, &_importDelimIndex);

    ftxui::InputOption custom_delim_option; // NOLINT(misc-const-correctness)
    custom_delim_option.cursor_position = static_cast<int>(_importCustomDelim.size());
    auto custom_delim_input = Input(&_importCustomDelim, ";", custom_delim_option);

    auto ignore_header_checkbox = Checkbox(txt::deck_editor::kImportIgnoreHeader, &_importIgnoreHeader);

    auto do_import = [this] {
        char resolved_delim = ';'; // NOLINT(misc-const-correctness)
        if (_importDelimIndex == 0) {
            resolved_delim = ',';
        } else if (_importDelimIndex == 1) {
            resolved_delim = ';';
        } else if (_importDelimIndex == 2) {
            resolved_delim = ':';
        } else if (_importDelimIndex == 3) {
            resolved_delim = '\t';
        } else if (_importDelimIndex == 4) {
            if (!_importCustomDelim.empty()) {
                resolved_delim = _importCustomDelim[0];
            } else {
                resolved_delim = ';';
            }
        }

        triggerImportRequested(_importPath, resolved_delim, _importIgnoreHeader);
        _isImporting = false;
    };

    auto import_confirm_btn = Button(txt::deck_editor::kImportConfirmButton, [do_import] { do_import(); }, btnStyle);
    auto import_cancel_btn = Button(txt::common::kCancelEscape, [this] { _isImporting = false; }, btnStyle);

    auto import_container = Container::Vertical({
        select_file_btn,
        delimiter_radiobox,
        custom_delim_input,
        ignore_header_checkbox,
        Container::Horizontal({import_confirm_btn, import_cancel_btn}),
    });

    auto import_renderer = Renderer(import_container, [this, select_file_btn, delimiter_radiobox, custom_delim_input, ignore_header_checkbox, import_confirm_btn, import_cancel_btn] {
        auto custom_delim_element = (_importDelimIndex == 4) 
            ? hbox({text(txt::deck_editor::kImportCustomDelimiterLabel), custom_delim_input->Render()})
            : text("");

        return vbox({
            text(txt::deck_editor::kImportTitle) | bold | color(Color::CyanLight) | center,
            blueSep(),
            hbox({text(txt::deck_editor::kImportFilePathLabel), text(" " + _importPath + " ") | color(Color::White) | flex}),
            select_file_btn->Render() | center,
            blueSep(),
            text(txt::deck_editor::kImportDelimiterLabel) | color(Color::Cornsilk1),
            delimiter_radiobox->Render(),
            custom_delim_element,
            blueSep(),
            ignore_header_checkbox->Render(),
            blueSep(),
            hbox({import_confirm_btn->Render(), text(" "), import_cancel_btn->Render()}) | center
        }) | border | bold | color(Color::BlueLight) | center | size(WIDTH, LESS_THAN, 70);
    });

    return CatchEvent(import_renderer, [this](const Event& event) {
        if (event == Event::Escape) {
            _isImporting = false;
            return true;
        }
        return false;
    });
}

void FtxuiDeckEditorView::run() {
    using namespace ftxui;
    using namespace app::ui;
    auto screen = ScreenInteractive::Fullscreen();
    
    _importDelimiterOptions = {",", ";", ":", "\\t", txt::deck_editor::kImportOptionOther};

    auto custom_btn_style = buttonStyle();

    ftxui::ButtonOption inline_btn_style = ftxui::ButtonOption::Animated(); // NOLINT(misc-const-correctness)
    inline_btn_style.transform = [](const ftxui::EntryState& state) {
        auto element = ftxui::text(std::string("[ ") + state.label + " ]") | ftxui::color(ftxui::Color::Cornsilk1);
        if (state.focused) {
            element = element | ftxui::inverted | ftxui::bold;
        }
        return element;
    };

    // --- Components & Actions ---
    auto add_card_action = [this] {
        if (!_vm.newFront.empty() && !_vm.newBack.empty() && triggerAddCard(_vm.newFront, _vm.newBack)) {
            _vm.clearNewCardBuffers();
        }
    };

    ftxui::InputOption input_front_opt; // NOLINT(misc-const-correctness)
    input_front_opt.cursor_position = static_cast<int>(_vm.newFront.size());
    auto input_front_base = Input(&_vm.newFront, txt::common::kFront, input_front_opt);

    ftxui::InputOption input_back_opt; // NOLINT(misc-const-correctness)
    input_back_opt.cursor_position = static_cast<int>(_vm.newBack.size());
    auto input_back_base = Input(&_vm.newBack, txt::common::kBack, input_back_opt);

    auto input_front = CatchEvent(input_front_base, [add_card_action](const Event& event) {
        if (event == Event::Return) {
            add_card_action();
            return true;
        }
        return false;
    });
    auto input_back = CatchEvent(input_back_base, [add_card_action](const Event& event) {
        if (event == Event::Return) {
            add_card_action();
            return true;
        }
        return false;
    });

    auto btn_add_card = Button(txt::deck_editor::kAddCardButton, add_card_action, custom_btn_style);

    auto btn_start_study = Button(txt::deck_editor::kStartSessionButton, [this, &screen] {
        triggerStartStudy();
        screen.Exit();
    }, custom_btn_style);

    auto btn_exit = Button(txt::common::kBackEscape, [this, &screen] {
        triggerExitToBrowser();
        screen.Exit();
    }, custom_btn_style);

    // --- Bulk Action Buttons ---
    auto btn_delete_bulk = Button(txt::deck_editor::kDeleteToolbarButton, [this] {
        if (_vm.getSelectedCount() > 0 || !_focusedCardId.empty()) { _isDeletingBulk = true; }
    }, custom_btn_style);

    auto btn_copy_bulk = Button(txt::deck_editor::kCopyToolbarButton, [this] {
        if (_vm.getSelectedCount() > 0 || !_focusedCardId.empty()) { _isCopyingBulk = true; }
    }, custom_btn_style);

    auto btn_move_bulk = Button(txt::deck_editor::kMoveToolbarButton, [this] {
        if (_vm.getSelectedCount() > 0 || !_focusedCardId.empty()) { _isMovingBulk = true; }
    }, custom_btn_style);

    auto btn_import = Button(txt::deck_editor::kImportToolbarButton, [this] {
        _isImporting = true;
    }, custom_btn_style);

    auto bulk_toolbar = Container::Horizontal({btn_delete_bulk, btn_copy_bulk, btn_move_bulk, btn_import});

    auto card_list_container = Container::Vertical({});
    
    _onDeckChangedInternal = [this, card_list_container, custom_btn_style]() {
        card_list_container->DetachAllChildren();

        if (_vm.deck) {
            auto cards = _vm.deck->getAllCards();
            for (const auto& card : cards) {
                _vm.cardSelectionState.try_emplace(card->id, false);

                auto checkbox = Checkbox("", &_vm.cardSelectionState[card->id]);
                auto row_container = Container::Horizontal({checkbox});

                auto row_with_edit = CatchEvent(row_container, [this, card, custom_btn_style](const Event& event) {
                    if (event == Event::Character("e") || event == Event::Character("E")) {
                        _vm.startEditing(card);
                        _isEditing = true;
                        _editModal = buildEditModal(custom_btn_style);
                        return true;
                    }
                    return false;
                });

                auto row_renderer = Renderer(row_with_edit, [this, row_with_edit, checkbox, card] {
                    const bool focused = row_with_edit->Focused();
                    if (focused) {
                        _focusedCardId = card->id;
                    }
                    auto row = hbox({
                        checkbox->Render(),
                        text(" "),
                        text(card->text_front) | color(Color::GreenLight) | size(WIDTH, EQUAL, 35),
                        separator() | bold | color(Color::BlueLight),
                        text(card->text_back) | color(Color::GreenLight) | flex
                    });
                    if (focused) {
                        row = row | bgcolor(Color::GrayDark) | color(Color::White) | bold;
                    }
                    return row;
                });

                card_list_container->Add(row_renderer);
            }
        }

        if (card_list_container->ChildCount() == 0) {
            card_list_container->Add(Renderer([] {
                return text(txt::deck_editor::kEmptyDeck) | center | color(Color::GrayLight);
            }));
        }
    };
    _onDeckChangedInternal(); // Trigger initial build

    auto main_container = Container::Vertical({
        bulk_toolbar,
        card_list_container,
        input_front,
        input_back,
        btn_add_card,
        btn_start_study,
        btn_exit,
    });
    main_container->SetActiveChild(btn_start_study.get());

    auto renderer = Renderer(main_container, [this, bulk_toolbar, card_list_container, input_front, input_back, btn_add_card, btn_start_study, btn_exit] {
        const std::string deck_name = _vm.deck ? _vm.deck->getName() : txt::common::kUnknown;
        const size_t card_count = _vm.deck ? _vm.deck->size() : 0;

        const size_t selected_count = _vm.getSelectedCount();

        auto header = vbox({
            text(txt::deck_editor::headerTitle(deck_name)) | bold | color(Color::CyanLight) | center,
            text(txt::deck_editor::headerStats(card_count, selected_count)) | color(Color::White) | center,
        });

        auto card_list_view = card_list_container->Render() | vscroll_indicator | yframe | flex;
        auto hint = text(txt::deck_editor::kSelectionHint) | color(Color::White) | center;

        return vbox({
            header,
            blueSep(),
            bulk_toolbar->Render() | center,
            blueSep(),
            card_list_view,
            blueSep(),
            hint,
            blueSep(),
            hbox({
                text(txt::deck_editor::kFrontFieldLabel) | color(Color::White), input_front->Render() | flex,
                text(txt::deck_editor::kBackFieldLabel) | color(Color::White), input_back->Render() | flex,
            }),
            btn_add_card->Render() | center,
            blueSep(),
            hbox({btn_start_study->Render(), text(" "), btn_exit->Render()}) | center
        }) | border | bold | color(Color::BlueLight) | center;
    });

    // --- Build modal components ---
    auto delete_modal = buildDeleteModal(custom_btn_style);
    auto move_modal   = buildMoveModal(custom_btn_style);
    auto copy_modal   = buildCopyModal(custom_btn_style);
    auto file_picker_modal = buildFilePickerModal(custom_btn_style);
    auto import_modal = buildImportModal(custom_btn_style, inline_btn_style);
    _editModal = buildEditModal(custom_btn_style);

    // --- Main Event Handler with Modal Overlay ---
    auto final_renderer = Renderer(renderer, [this, renderer, delete_modal, move_modal, copy_modal, file_picker_modal, import_modal, custom_btn_style] {
        auto base = renderer->Render();

        if (_isEditing) {
            if (!_editModal) {
                _editModal = buildEditModal(custom_btn_style);
            }
            return dbox({base | dim, _editModal->Render() | clear_under | center});
        }
        if (_isDeletingBulk) {
            return dbox({base | dim, delete_modal->Render() | clear_under | center});
        }
        if (_isMovingBulk) {
            return dbox({base | dim, move_modal->Render() | clear_under | center});
        }
        if (_isCopyingBulk) {
            return dbox({base | dim, copy_modal->Render() | clear_under | center});
        }
        if (_isFilePickerActive) {
            return dbox({base | dim, file_picker_modal->Render() | clear_under | center});
        }
        if (_isImporting) {
            return dbox({base | dim, import_modal->Render() | clear_under | center});
        }
        return base;
    });

    auto event_handler = CatchEvent(final_renderer, [this, &screen, delete_modal, move_modal, copy_modal, file_picker_modal, import_modal, input_front, input_back, custom_btn_style](Event event) {

        if (event.is_mouse() && event.mouse().button == ftxui::Mouse::None) {
            return true;
        }

        if (_isEditing) {
            if (!_editModal) {
                _editModal = buildEditModal(custom_btn_style);
            }
            _editModal->OnEvent(event);
            return true;
        }
        if (_isDeletingBulk) {
            delete_modal->OnEvent(event);
            return true;
        }
        if (_isMovingBulk) {
            move_modal->OnEvent(event);
            return true;
        }
        if (_isCopyingBulk) {
            copy_modal->OnEvent(event);
            return true;
        }
        if (_isFilePickerActive) {
            file_picker_modal->OnEvent(event);
            return true;
        }
        if (_isImporting) {
            import_modal->OnEvent(event);
            return true;
        }

        // Only allow single-character shortcuts if the user isn't typing in the Add Card inputs
        const bool isTyping = input_front->Focused() || input_back->Focused();
        
        if (!isTyping) {
            if (event == Event::Character("s") || event == Event::Character("S")) {
                triggerStartStudy();
                screen.Exit();
                return true;
            }
            if (event == Event::Character("u") || event == Event::Character("U")) {
                if (_vm.getSelectedCount() > 0 || !_focusedCardId.empty()) { _isDeletingBulk = true; return true; }
            }
            if (event == Event::Character("k") || event == Event::Character("K")) {
                if (_vm.getSelectedCount() > 0 || !_focusedCardId.empty()) { _isCopyingBulk = true; return true; }
            }
            if (event == Event::Character("p") || event == Event::Character("P")) {
                if (_vm.getSelectedCount() > 0 || !_focusedCardId.empty()) { _isMovingBulk = true; return true; }
            }
            if (event == Event::Character("i") || event == Event::Character("I")) {
                _isImporting = true;
                return true;
            }
        }
        if (event == Event::Escape) {
            triggerExitToBrowser();
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(event_handler);
}

} // namespace app
