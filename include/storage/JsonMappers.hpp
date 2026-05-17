#pragma once

#include "core/CardId.hpp"
#include "core/Flashcard.hpp"
#include "core/FlashcardList.hpp"
#include "storage/JsonSchema.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <memory>
#include <unordered_set>

// Placing to_json and from_json in the same namespace as the target types
// allows nlohmann::json to automatically discover them via Argument-Dependent Lookup (ADL).
namespace core {

    inline void to_json(nlohmann::json& j, const CardState& state) {
        if (state == CardState::Known) {
            j = json_values::kStateKnown;
        } else if (state == CardState::Mastered) {
            j = json_values::kStateMastered;
        } else {
            j = json_values::kStateNew; // Default to New for unknown states
        }
    }

    inline void from_json(const nlohmann::json& j, CardState& state) {
        if (!j.is_string()) {
            throw std::runtime_error("CardState must be a string.");
        }
        std::string s = j.get<std::string>();
        if (s == json_values::kStateNew) {
            state = CardState::New;
        } else if (s == json_values::kStateKnown) {
            state = CardState::Known;
        } else if (s == json_values::kStateMastered) {
            state = CardState::Mastered;
        } else {
            throw std::runtime_error("Unknown CardState string: " + s);
        }
    }

    inline void to_json(nlohmann::json& j, const Flashcard& card) {
        j[json_keys::kId] = card.card_id;
        j[json_keys::kTextFront] = card.text_front;
        j[json_keys::kTextBack] = card.text_back;
        to_json(j[json_keys::kStateFrontToBack], card.state_Front_to_Back);
        to_json(j[json_keys::kStateBackToFront], card.state_Back_to_Front);
    }

    inline void from_json(const nlohmann::json& j, Flashcard& card) {
        if (j.contains(json_keys::kId) && j.at(json_keys::kId).is_string()) {
            j.at(json_keys::kId).get_to(card.card_id);
        } else {
            card.card_id.clear();
        }
        j.at(json_keys::kTextFront).get_to(card.text_front);
        j.at(json_keys::kTextBack).get_to(card.text_back);

        if (j.contains(json_keys::kStateFrontToBack)) {
            from_json(j.at(json_keys::kStateFrontToBack), card.state_Front_to_Back);
        } else {
            card.state_Front_to_Back = CardState::New;
        }

        if (j.contains(json_keys::kStateBackToFront)) {
            from_json(j.at(json_keys::kStateBackToFront), card.state_Back_to_Front);
        } else {
            card.state_Back_to_Front = CardState::New;
        }
    }

    inline void to_json(nlohmann::json& j, const FlashcardList& list) {
        j[json_keys::kName] = list.getName();
        nlohmann::json cards_array = nlohmann::json::array();
        for (const auto& card_ptr : list.getAllCards()) { // Use getAllCards to get all cards
            if (card_ptr) {
                nlohmann::json card_json;
                to_json(card_json, *card_ptr);
                cards_array.push_back(card_json);
            }
        }
        j[json_keys::kCards] = cards_array;
    }

    inline void from_json(const nlohmann::json& j, FlashcardList& list) {
        if (!j.contains(json_keys::kName) || !j.contains(json_keys::kCards)) {
            throw std::runtime_error("Invalid FlashcardList JSON format.");
        }

        list.setName(j.at(json_keys::kName).get<std::string>());
        list.clear(); // Clear existing cards before importing
        std::unordered_set<std::string> usedIds;

        if (j.at(json_keys::kCards).is_array()) {
            for (const auto& card_json : j.at(json_keys::kCards)) {
                Flashcard card_data;
                from_json(card_json, card_data);

                if (card_data.card_id.empty() || usedIds.contains(card_data.card_id)) {
                    card_data.card_id = core::generateUniqueCardId(usedIds);
                } else {
                    usedIds.insert(card_data.card_id);
                }

                list.addCard(std::make_shared<Flashcard>(std::move(card_data)));
            }
        } else {
            throw std::runtime_error("FlashcardList 'cards' must be an array.");
        }
    }

} // namespace core