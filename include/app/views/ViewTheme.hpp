#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

/// @file ViewTheme.hpp
/// Central style/color definitions shared across all Ftxui view files.
/// Import with:  using namespace app::ui;

namespace app::ui {

// ─── Color palette ────────────────────────────────────────────────────────────

/// Primary border / separator / frame color.
inline const ftxui::Color BorderColor   = ftxui::Color::BlueLight;
/// Button label text color.
inline const ftxui::Color ButtonColor   = ftxui::Color::Cornsilk1;
/// Card front/back text color.
inline const ftxui::Color CardTextColor = ftxui::Color::LightSeaGreen;
/// Section header / label color.
inline const ftxui::Color HeaderColor   = ftxui::Color::CyanLight;

// ─── Element helpers ──────────────────────────────────────────────────────────

/// Bold blue-themed separator — use instead of `separator() | bold | color(Color::BlueLight)`.
inline ftxui::Element blueSep() {
    return ftxui::separator() | ftxui::bold | ftxui::color(BorderColor);
}

// ─── Component helpers ────────────────────────────────────────────────────────

/// Standard button style: Cornsilk1 bordered label, inverted when focused.
/// Use instead of the repeated ButtonOption::Animated() + transform block.
inline ftxui::ButtonOption buttonStyle() {
    ftxui::ButtonOption style = ftxui::ButtonOption::Animated();
    style.transform = [](const ftxui::EntryState& s) {
        auto element = ftxui::text(s.label)
                     | ftxui::border
                     | ftxui::bold
                     | ftxui::color(ButtonColor);
        if (s.focused) element = element | ftxui::inverted;
        return element;
    };
    return style;
}

/// Wraps a component to silently discard mouse-hover events (prevents UI lag).
inline ftxui::Component drainMouseHover(ftxui::Component inner) {
    return ftxui::CatchEvent(std::move(inner), [](ftxui::Event event) {
        return event.is_mouse() && event.mouse().button == ftxui::Mouse::None;
    });
}

} // namespace app::ui
