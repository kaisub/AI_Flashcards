#include "app/views/FtxuiStudyConfigView.hpp"
#include "app/localization/Localization.hpp"
#include "app/views/ViewTheme.hpp"
#include "app/views/ViewUtils.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

namespace app {

namespace txt = ::app::localization::selected;

void FtxuiStudyConfigView::run() {
    using namespace ftxui;
    using namespace app::ui;
    auto screen = ScreenInteractive::Fullscreen();

    // Direction options
    const std::vector<std::string> direction_entries = {
        txt::study_config::kDirectionFrontToBack,
        txt::study_config::kDirectionBackToFront,
        txt::study_config::kDirectionMixed
    };

    // Mode options
    const std::vector<std::string> mode_entries = {
        txt::study_config::kModeStandard,
        txt::study_config::kModeFocused
    };

    // Target state options (for Focused mode)
    const std::vector<std::string> target_state_entries = {
        txt::study_config::kTargetNew,
        txt::study_config::kTargetKnown,
        txt::study_config::kTargetMastered
    };

    auto direction_radiobox = Radiobox(&direction_entries, &_vm.directionSelected);
    auto mode_radiobox = Radiobox(&mode_entries, &_vm.modeSelected);
    auto target_state_radiobox = Radiobox(&target_state_entries, &_vm.targetStateSelected);

    auto slider_new = Slider("", &_vm.weightNew, 0, 100, 1);
    auto slider_known = Slider("", &_vm.weightKnown, 0, 100, 1);
    auto slider_mastered = Slider("", &_vm.weightMastered, 0, 100, 1);

    auto start_action = [this, &screen] {
        const core::SessionConfig config = [this] {
            core::SessionConfig cfg; // NOLINT(misc-const-correctness)
            cfg.direction = _vm.getSelectedDirection();
            cfg.type = _vm.getSelectedMode();
            cfg.focusedTargetState = (cfg.type == core::SessionType::Focused)
                ? std::optional<core::CardState>(_vm.getSelectedTargetState())
                : std::nullopt;
            cfg.weightNew = static_cast<unsigned int>(_vm.weightNew);
            cfg.weightKnown = static_cast<unsigned int>(_vm.weightKnown);
            cfg.weightMastered = static_cast<unsigned int>(_vm.weightMastered);
            return cfg;
        }();

        triggerStart(config);
        screen.ExitLoopClosure()();
    };

    // Match DeckEditor button styling.
    auto custom_btn_style = buttonStyle();

    // Create buttons with custom styling
    auto start_button = Button(txt::study_config::kStartButton, start_action, custom_btn_style);

    auto cancel_button = Button(txt::study_config::kCancelButton, [this, &screen] {
        triggerCancel();
        screen.ExitLoopClosure()();
    }, custom_btn_style);

    auto button_container = Container::Horizontal(Components{
        start_button,
        cancel_button
    });

    // Create the complete interactive container with ALL components including the 3 sliders
    auto component = Container::Vertical(Components{
        direction_radiobox,
        mode_radiobox,
        target_state_radiobox,
        slider_new,
        slider_known,
        slider_mastered,
        button_container
    });

    // Set initial focus on the Start button
    component->SetActiveChild(button_container.get());
    button_container->SetActiveChild(start_button.get());

    // Renderer that draws all elements including sliders with sum indicator and buttons at bottom
    auto renderer = Renderer(component, [&] {
        _vm.balanceWeights();

        const bool is_focused = (_vm.modeSelected == 1);
        auto focused_label_color = is_focused ? Color::CyanLight : Color::GrayDark;
        auto target_state_element = target_state_radiobox->Render();
        if (is_focused) {
            target_state_element = target_state_element | color(Color::LightSeaGreen);
        } else {
            target_state_element = target_state_element | color(Color::GrayDark);
        }

        return vbox(Elements{
            text(txt::study_config::kTitle) | bold | color(Color::CyanLight) | center,
            blueSep(),
            text(txt::study_config::kDirectionLabel) | bold | color(Color::CyanLight),
            direction_radiobox->Render() | color(Color::LightSeaGreen),
            text(txt::study_config::kModeLabel) | bold | color(Color::CyanLight),
            mode_radiobox->Render() | color(Color::LightSeaGreen),
            text(txt::study_config::kTargetLabel) | bold | color(focused_label_color),
            target_state_element,
            blueSep(),
            text(txt::study_config::kWeightsLabel) | bold | color(Color::CyanLight),
            vbox(Elements{
                hbox(Elements{ text(txt::study_config::weightNewLabel(_vm.weightNew)) | size(WIDTH, EQUAL, 18), slider_new->Render() | flex }) | color(Color::LightSeaGreen),
                hbox(Elements{ text(txt::study_config::weightKnownLabel(_vm.weightKnown)) | size(WIDTH, EQUAL, 18), slider_known->Render() | flex }) | color(Color::LightSeaGreen),
                hbox(Elements{ text(txt::study_config::weightMasteredLabel(_vm.weightMastered)) | size(WIDTH, EQUAL, 18), slider_mastered->Render() | flex }) | color(Color::LightSeaGreen),
            }) | border | bold | color(Color::BlueLight),
            blueSep(),
            hbox(Elements{
                start_button->Render() | flex,
                text("  "),
                cancel_button->Render() | flex
            }) | center
        }) | border | bold | color(Color::BlueLight) | size(WIDTH, GREATER_THAN, 70) | center;
    });

    auto event_handler = CatchEvent(renderer, [this, &screen, start_action](Event event) {
        // Discard mouse movement events to prevent UI lag
        if (event.is_mouse() && event.mouse().button == Mouse::None) {
            return true;
        }

        if (app::views::utils::isCharInsensitive(event, txt::common::kStartSession)) {
            start_action();
            return true;
        }

        if (app::views::utils::isEscape(event)) {
            triggerCancel();
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    screen.Loop(event_handler);
}

} // namespace app
