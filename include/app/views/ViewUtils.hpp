#pragma once

#include <string>
#include <algorithm>
#include <cctype>

namespace app::views::utils {

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