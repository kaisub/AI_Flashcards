#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <ftxui/component/event.hpp>

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