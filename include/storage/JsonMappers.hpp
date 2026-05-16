#pragma once

#include "core/Flashcard.hpp"
#include "core/FlashcardList.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <memory>

// Placing to_json and from_json in the same namespace as the target types
// allows nlohmann::json to automatically discover them via Argument-Dependent Lookup (ADL).
namespace core {

    inline void to_json(nlohmann::json& j, const CardState& state) {
        if (state == CardState::Known) {
            j = "Known";
        } else if (state == CardState::Mastered) {
            j = "Mastered";
        } else {
            j = "New"; // Default to New for unknown states
        }
    }

    inline void from_json(const nlohmann::json& j, CardState& state) {
        if (!j.is_string()) {
            throw std::runtime_error("CardState must be a string.");
        }
        std::string s = j.get<std::string>();
        if (s == "New") {
            state = CardState::New;
        } else if (s == "Known") {
            state = CardState::Known;
        } else if (s == "Mastered") {
            state = CardState::Mastered;
        } else {
            throw std::runtime_error("Unknown CardState string: " + s);
        }
    }

    inline void to_json(nlohmann::json& j, const Flashcard& card) {
        j["id"] = card.card_id;
        j["text_front"] = card.text_front;
        j["text_back"] = card.text_back;
        to_json(j["state_Front_to_Back"], card.state_Front_to_Back);
        to_json(j["state_Back_to_Front"], card.state_Back_to_Front);
    }

    inline void from_json(const nlohmann::json& j, Flashcard& card) {
        j.at("id").get_to(card.card_id);
        j.at("text_front").get_to(card.text_front);
        j.at("text_back").get_to(card.text_back);
        from_json(j.at("state_Front_to_Back"), card.state_Front_to_Back);
        from_json(j.at("state_Back_to_Front"), card.state_Back_to_Front);
    }

    inline void to_json(nlohmann::json& j, const FlashcardList& list) {
        j["name"] = list.getName();
        nlohmann::json cards_array = nlohmann::json::array();
        for (const auto& card_ptr : list.getAllCards()) { // Use getAllCards to get all cards
            if (card_ptr) {
                nlohmann::json card_json;
                to_json(card_json, *card_ptr);
                cards_array.push_back(card_json);
            }
        }
        j["cards"] = cards_array;
    }

    inline void from_json(const nlohmann::json& j, FlashcardList& list) {
        if (!j.contains("name") || !j.contains("cards")) {
            throw std::runtime_error("Invalid FlashcardList JSON format.");
        }

        list.setName(j.at("name").get<std::string>());
        list.clear(); // Clear existing cards before importing

        if (j.at("cards").is_array()) {
            for (const auto& card_json : j.at("cards")) {
                Flashcard card_data;
                from_json(card_json, card_data);
                list.addCard(std::make_shared<Flashcard>(std::move(card_data)));
            }
        } else {
            throw std::runtime_error("FlashcardList 'cards' must be an array.");
        }
    }

} // namespace core