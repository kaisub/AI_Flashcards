#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <functional>
#include <memory>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

namespace app::views::utils {

    inline bool isEscape(const ftxui::Event& event) {
        return event == ftxui::Event::Escape;
    }

    inline bool isCharInsensitive(const ftxui::Event& event, char c) {
        const auto lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        const auto upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return event == ftxui::Event::Character(std::string(1, lower)) ||
               event == ftxui::Event::Character(std::string(1, upper));
    }

    inline bool isAltCharInsensitive(const ftxui::Event& event, char c) {
        constexpr const char* kAltPrefix = "\x1B";
        const auto lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        const auto upper = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return event == ftxui::Event::Special(std::string(kAltPrefix) + std::string(1, lower)) ||
               event == ftxui::Event::Special(std::string(kAltPrefix) + std::string(1, upper));
    }

    // Shared list behavior: first click selects an item, second click on the same selected item activates it.
    inline bool handleMenuTwoClickMouseSelect(
        ftxui::Event event,
        const ftxui::Component& menu,
        const std::shared_ptr<ftxui::Box>& menu_box,
        int& selected_index,
        std::size_t entry_count,
        bool& click_armed,
        int& last_clicked_index,
        const std::function<void()>& activate_selected) {
        if (event.is_mouse() && event.mouse().button == ftxui::Mouse::None) {
            return true;
        }

        if (!event.is_mouse() || event.mouse().button != ftxui::Mouse::Left) {
            return false;
        }

        const auto& mouse = event.mouse();
        const bool inside_menu = menu_box->Contain(mouse.x, mouse.y);

        auto is_item_row = [&]() {
            const int local_y = mouse.y - menu_box->y_min;
            const int visible_rows = menu_box->y_max - menu_box->y_min + 1;
            const int used_rows = std::min<int>(visible_rows, static_cast<int>(entry_count));
            return local_y >= 0 && local_y < used_rows;
        };

        if (event.mouse().motion == ftxui::Mouse::Pressed) {
            if (!inside_menu) {
                click_armed = false;
                last_clicked_index = -1;
                return false;
            }

            menu->OnEvent(event);
            click_armed = is_item_row();
            return true;
        }

        if (event.mouse().motion == ftxui::Mouse::Released) {
            if (!inside_menu) {
                click_armed = false;
                last_clicked_index = -1;
                return false;
            }

            menu->OnEvent(event);
            const bool clicked_item_row = is_item_row();
            if (click_armed && clicked_item_row) {
                if (last_clicked_index == selected_index) {
                    last_clicked_index = -1;
                    click_armed = false;
                    activate_selected();
                    return true;
                }
                last_clicked_index = selected_index;
            }

            click_armed = false;
            return true;
        }

        return false;
    }

    // Helper function to trim whitespace from both ends of a string
    inline std::string trim(const std::string& str) {
        auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
            return std::isspace(ch);
        });
        auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
            return std::isspace(ch);
        }).base();
        return (start < end) ? std::string(start, end) : std::string();
    }

    // Helper function to convert a string to lowercase
    inline std::string toLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    // Helper function to validate user input against the expected back of the card
    inline bool isAnswerCorrect(const std::string& userInput, const std::string& expectedAnswer) {
        std::string trimmed_input = trim(userInput);
        std::string trimmed_expected = trim(expectedAnswer);
        
        return toLower(trimmed_input) == toLower(trimmed_expected);
    }

} // namespace app::views::utils