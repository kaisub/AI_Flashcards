#pragma once

namespace core::json_keys {
    inline constexpr const char* kId = "id";
    inline constexpr const char* kName = "name";
    inline constexpr const char* kCards = "cards";
    inline constexpr const char* kTextFront = "text_front";
    inline constexpr const char* kTextBack = "text_back";
    inline constexpr const char* kStateFrontToBack = "state_Front_to_Back";
    inline constexpr const char* kStateBackToFront = "state_Back_to_Front";
} // namespace core::json_keys

namespace core::json_values {
    inline constexpr const char* kStateNew = "New";
    inline constexpr const char* kStateKnown = "Known";
    inline constexpr const char* kStateMastered = "Mastered";
} // namespace core::json_values
