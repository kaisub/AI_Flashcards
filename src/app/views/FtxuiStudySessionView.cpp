#include "app/views/FtxuiStudySessionView.hpp"
#include "app/localization/Localization.hpp"
#include "app/views/ViewTheme.hpp"
#include "app/views/ViewUtils.hpp"
#include <algorithm>
#include <cctype>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <utility>
#include <vector>

namespace app {

namespace txt = ::app::localization::selected;

void FtxuiStudySessionView::showCard(const core::Flashcard& card, bool showBack) {
    _vm.setCard(card, showBack);
}

void FtxuiStudySessionView::showSessionComplete() {
    _vm.isComplete = true;
}

void FtxuiStudySessionView::setAvailableLists(const std::vector<std::string>& listNames) {
    _availableLists = listNames;
    _selectedListIndex = 0; // Reset selection
}

void FtxuiStudySessionView::run() {
    using namespace ftxui;
    using namespace app::ui;
    int focused_button_index = 0;
    auto screen = ScreenInteractive::Fullscreen();
    bool return_to_controller = false;
    while (!return_to_controller) {
        Component main_component;
        if (_vm.isComplete) {
            main_component = buildSessionCompleteView(screen, return_to_controller);
        } else if (_isDeleting) {
            main_component = buildDeleteModal(screen, return_to_controller);
        } else if (_isEditing) {
            main_component = buildEditModal(screen);
        } else if (_isCopying) {
            main_component = buildCopyModal(screen);
        } else {
            main_component = buildCardView(screen, return_to_controller, focused_button_index);
        }
        screen.Loop(drainMouseHover(main_component));
    }
}

ftxui::Component FtxuiStudySessionView::buildSessionCompleteView(ftxui::ScreenInteractive& screen, bool& returnToController) {
    using namespace ftxui;
    using namespace app::ui;

    auto exit_button = Button(txt::study_session::kFinishButton, [this, &screen, &returnToController] {
        returnToController = true;
        triggerExitRequested();
        screen.Exit();
    });

    return Renderer(exit_button, [exit_button] {
        return vbox({
                   text(txt::study_session::kCompleteTitle) | hcenter | vcenter | flex,
                   blueSep(),
                   exit_button->Render() | hcenter,
               }) | border | bold | color(Color::BlueLight);
    });
}

ftxui::Component FtxuiStudySessionView::buildDeleteModal(ftxui::ScreenInteractive& screen, bool& returnToController) {
    using namespace ftxui;
    using namespace app::ui;

    auto yes_button = Button(txt::common::kYesEnter, [this, &screen, &returnToController] {
        triggerDeleteRequested();
        _isDeleting = false;
        returnToController = true;
        screen.Exit();
    });

    auto no_button = Button(txt::common::kNoEscape, [this, &screen] {
        _isDeleting = false;
        screen.Exit();
    });

    auto delete_container = Container::Horizontal({yes_button, no_button});
    delete_container->SetActiveChild(yes_button.get());

    auto delete_renderer = Renderer(delete_container, [yes_button, no_button] {
        return vbox({
            text(txt::study_session::kDeletePrompt) | bold | center,
            blueSep(),
            text(""),
            hbox({
                yes_button->Render() | flex,
                text("  "),
                no_button->Render() | flex
            }) | center
        }) | border | bold | color(Color::BlueLight) | center | size(WIDTH, GREATER_THAN, 50);
    });

    return CatchEvent(delete_renderer, [this, &screen](const Event& event) {
        if (event == Event::Escape) {
            _isDeleting = false;
            screen.Exit();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiStudySessionView::buildEditModal(ftxui::ScreenInteractive& screen) {
    using namespace ftxui;
    using namespace app::ui;

    InputOption front_option;
    front_option.cursor_position = static_cast<int>(_vm.editFront.size());
    auto input_front = Input(&_vm.editFront, txt::common::kFront, front_option);

    InputOption back_option;
    back_option.cursor_position = static_cast<int>(_vm.editBack.size());
    auto input_back = Input(&_vm.editBack, txt::common::kBack, back_option);

    auto save_action = [this, &screen] {
        triggerEditRequested(_vm.editFront, _vm.editBack);
        _isEditing = false;
        screen.Exit();
    };

    auto save_button = Button(txt::common::kSaveEnter, save_action);
    auto cancel_button = Button(txt::common::kCancelEscape, [this, &screen] {
        _isEditing = false;
        screen.Exit();
    });

    auto edit_container = Container::Vertical({
        input_front,
        input_back,
        Container::Horizontal({save_button, cancel_button})
    });
    edit_container->SetActiveChild(input_front.get());

    auto edit_renderer = Renderer(edit_container, [input_front, input_back, save_button, cancel_button] {
        return vbox({
            text(txt::study_session::kEditTitle) | bold | center,
            blueSep(),
            text(""),
            text(txt::common::kFrontLabel) | bold,
            input_front->Render(),
            text(""),
            text(txt::common::kBackLabel) | bold,
            input_back->Render(),
            text(""),
            blueSep(),
            hbox({
                save_button->Render() | flex,
                text("  "),
                cancel_button->Render() | flex
            })
        }) | border | bold | color(Color::BlueLight) | center | size(WIDTH, GREATER_THAN, 60);
    });

    return CatchEvent(edit_renderer, [this, &screen, save_action](const Event& event) {
        if (event == Event::Escape) {
            _isEditing = false;
            screen.Exit();
            return true;
        }
        if (event == Event::Return) {
            save_action();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiStudySessionView::buildCopyModal(ftxui::ScreenInteractive& screen) {
    using namespace ftxui;
    using namespace app::ui;

    MenuOption list_option;
    auto copy_action = [this, &screen] {
        triggerCopyRequested(_selectedListIndex);
        _isCopying = false;
        screen.Exit();
    };
    list_option.on_enter = copy_action;
    auto list_menu = Menu(&_availableLists, &_selectedListIndex, list_option);

    auto copy_button = Button(txt::common::kCopyEnter, copy_action);
    auto cancel_button = Button(txt::common::kCancelEscape, [this, &screen] {
        _isCopying = false;
        screen.Exit();
    });

    auto copy_container = Container::Vertical({
        list_menu,
        Container::Horizontal({copy_button, cancel_button})
    });
    copy_container->SetActiveChild(list_menu.get());

    auto copy_renderer = Renderer(copy_container, [list_menu, copy_button, cancel_button] {
        return vbox({
            text(txt::study_session::kCopyTitle) | bold | center,
            blueSep(),
            text(""),
            text(txt::study_session::kChooseListLabel) | bold,
            list_menu->Render() | frame | size(HEIGHT, LESS_THAN, 15),
            text(""),
            blueSep(),
            hbox({
                copy_button->Render() | flex,
                text("  "),
                cancel_button->Render() | flex
            })
        }) | border | bold | color(Color::BlueLight) | center | size(WIDTH, GREATER_THAN, 50);
    });

    return CatchEvent(copy_renderer, [this, &screen](const Event& event) {
        if (event == Event::Escape) {
            _isCopying = false;
            screen.Exit();
            return true;
        }
        return false;
    });
}

ftxui::Component FtxuiStudySessionView::buildCardView(ftxui::ScreenInteractive& screen, bool& returnToController, int& focusedButtonIndex) {
    using namespace ftxui;
    using namespace app::ui;

    auto custom_btn_style = buttonStyle();

    InputOption answer_option;
    answer_option.transform = [this](const InputState& state) -> Element {
        if (_vm.lastCheckResult == app::model::CheckResult::Correct) {
            return state.element | color(Color::Black) | bgcolor(Color::Green);
        }
        if (_vm.lastCheckResult == app::model::CheckResult::Incorrect) {
            return state.element | color(Color::White) | bgcolor(Color::Red);
        }
        return state.element | bgcolor(Color::GrayDark);
    };

    auto exit_button = Button(txt::study_session::kExitButton, [this, &screen, &returnToController, &focusedButtonIndex] {
        focusedButtonIndex = 0;
        returnToController = true;
        triggerExitRequested();
        screen.Exit();
    }, custom_btn_style);
    auto undo_button = Button(txt::study_session::kUndoButton, [this, &screen, &returnToController, &focusedButtonIndex] {
        focusedButtonIndex = 1;
        returnToController = true;
        triggerUndoRequested();
        screen.Exit();
    }, custom_btn_style);
    auto rate_new_button = Button(txt::study_session::kRateNewButton, [this, &screen, &returnToController, &focusedButtonIndex] {
        focusedButtonIndex = 2;
        returnToController = true;
        triggerCardRated(core::CardState::New);
        screen.Exit();
    }, custom_btn_style);
    auto rate_known_button = Button(txt::study_session::kRateKnownButton, [this, &screen, &returnToController, &focusedButtonIndex] {
        focusedButtonIndex = 3;
        returnToController = true;
        triggerCardRated(core::CardState::Known);
        screen.Exit();
    }, custom_btn_style);
    auto rate_mastered_button = Button(txt::study_session::kRateMasteredButton, [this, &screen, &returnToController, &focusedButtonIndex] {
        focusedButtonIndex = 4;
        returnToController = true;
        triggerCardRated(core::CardState::Mastered);
        screen.Exit();
    }, custom_btn_style);

    auto button_bar = Container::Horizontal({
        exit_button, undo_button, rate_new_button, rate_known_button, rate_mastered_button,
    });

    if (focusedButtonIndex == 1) {
        button_bar->SetActiveChild(undo_button.get());
    } else if (focusedButtonIndex == 2) {
        button_bar->SetActiveChild(rate_new_button.get());
    } else if (focusedButtonIndex == 3) {
        button_bar->SetActiveChild(rate_known_button.get());
    } else if (focusedButtonIndex == 4) {
        button_bar->SetActiveChild(rate_mastered_button.get());
    } else {
        button_bar->SetActiveChild(exit_button.get());
    }

    auto answer_input = Input(&_vm.userInput, txt::study_session::kAnswerPlaceholder, answer_option);
    auto bottom_container = Container::Vertical({answer_input, button_bar});

    auto renderer = Renderer(bottom_container, [this, answer_input, button_bar] {
        Elements text_content_elements;
        text_content_elements.push_back(text(" "));
        text_content_elements.push_back(text(" "));
        text_content_elements.push_back(text(" "));
        text_content_elements.push_back(text(" "));
        text_content_elements.push_back(text(_vm.currentFront) | bold | hcenter | color(Color::LightSeaGreen));
        if (_vm.isFlipped) {
            text_content_elements.push_back(text(" "));
            text_content_elements.push_back(text(" "));
            text_content_elements.push_back(blueSep());
            text_content_elements.push_back(text(" "));
            text_content_elements.push_back(text(" "));
            text_content_elements.push_back(text(_vm.currentBack) | bold | hcenter | color(Color::LightSeaGreen));
        }
        text_content_elements.push_back(text(" "));
        text_content_elements.push_back(text(" "));
        text_content_elements.push_back(text(" "));
        text_content_elements.push_back(text(" "));

        auto text_content = vbox(std::move(text_content_elements)) | vcenter | flex;
        auto card_area = vbox({
            text_content,
            answer_input->Render() | size(WIDTH, EQUAL, 80) | hcenter
        }) | border | bold | color(Color::BlueLight) | flex;

        std::string status_text;
        switch (_vm.currentCardState) {
            case core::CardState::New:      status_text = txt::common::kStatusNew; break;
            case core::CardState::Known:    status_text = txt::common::kStatusKnown; break;
            case core::CardState::Mastered: status_text = txt::common::kStatusMastered; break;
        }

        auto hint_text = hbox({
            text(txt::study_session::kHint) | dim,
            text("  "),
            text(status_text) | bold | color(Color::Cyan)
        }) | hcenter;

        return vbox({
            card_area,
            blueSep(),
            hint_text,
            text(" "),
            blueSep(),
            button_bar->Render() | hcenter,
        });
    });

    return CatchEvent(renderer, [this, &screen, &returnToController, &focusedButtonIndex, button_bar](const Event& event) {
        auto is_alt_char = [&](char c) {
            const std::string lower = std::string("\x1B") + std::string(1, c);
            const std::string upper = std::string("\x1B") + std::string(1, static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
            return event == Event::Special(lower) || event == Event::Special(upper);
        };

        auto rate_card = [&](int focus_idx, core::CardState state) {
            focusedButtonIndex = focus_idx;
            returnToController = true;
            triggerCardRated(state);
            screen.Exit();
            return true;
        };

        if (event == Event::ArrowUp || event == Event::Return) {
            // If a user tabbed to the bottom buttons, let them press Enter natively
            if (event == Event::Return && button_bar->Focused()) {
                return false; 
            }
            if (!_vm.isFlipped) {
                const bool isCorrect = app::views::utils::isAnswerCorrect(_vm.userInput, _vm.currentBack);
                _vm.lastCheckResult = isCorrect ? app::model::CheckResult::Correct : app::model::CheckResult::Incorrect;
            }
            _vm.isFlipped = !_vm.isFlipped;
            return true;
        }
        if (event == Event::Escape) {
            focusedButtonIndex = 0;
            returnToController = true;
            triggerExitRequested();
            screen.Exit();
            return true;
        }
        if (is_alt_char('b')) {
            focusedButtonIndex = 1;
            returnToController = true;
            triggerUndoRequested();
            screen.Exit();
            return true;
        }
        
        if (is_alt_char('1') || event == Event::ArrowLeft)  { return rate_card(2, core::CardState::New); }
        if (is_alt_char('2') || event == Event::ArrowDown)   { return rate_card(3, core::CardState::Known); }
        if (is_alt_char('3') || event == Event::ArrowRight)  { return rate_card(4, core::CardState::Mastered); }

        if (is_alt_char('e')) {
            _isEditing = true;
            screen.Exit();
            return true;
        }
        if (is_alt_char('d')) {
            _isDeleting = true;
            screen.Exit();
            return true;
        }
        if (is_alt_char('c')) {
            _isCopying = true;
            screen.Exit();
            return true;
        }
        return false;
    });
}

} // namespace app
