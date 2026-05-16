#include <gtest/gtest.h>
#include "storage/JsonMappers.hpp"
#include "core/Flashcard.hpp"
#include "core/FlashcardList.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;
using namespace core;

// --- CardState Tests ---

TEST(JsonMappersTest, CardStateSerialization) {
    json j1, j2, j3, j4;
    to_json(j1, CardState::New);
    to_json(j2, CardState::Known);
    to_json(j3, CardState::Mastered);
    
    // Edge case: invalid enum value should default to "New"
    to_json(j4, static_cast<CardState>(999));

    EXPECT_EQ(j1, "New");
    EXPECT_EQ(j2, "Known");
    EXPECT_EQ(j3, "Mastered");
    EXPECT_EQ(j4, "New");
}

TEST(JsonMappersTest, CardStateDeserialization) {
    CardState state = CardState::New;
    
    json j1 = "New";
    from_json(j1, state);
    EXPECT_EQ(state, CardState::New);

    json j2 = "Known";
    from_json(j2, state);
    EXPECT_EQ(state, CardState::Known);

    json j3 = "Mastered";
    from_json(j3, state);
    EXPECT_EQ(state, CardState::Mastered);
}

TEST(JsonMappersTest, CardStateDeserializationErrors) {
    CardState state = CardState::New;
    
    // Edge case: Unrecognized string
    json jInvalidString = "SlightlyKnown";
    EXPECT_THROW(from_json(jInvalidString, state), std::runtime_error);

    // Edge case: Wrong JSON type
    json jInvalidType = 123;
    EXPECT_THROW(from_json(jInvalidType, state), std::runtime_error);
}

// --- Flashcard Tests ---

TEST(JsonMappersTest, FlashcardSerialization) {
    Flashcard card{"123", "Front Text", "Back Text", CardState::Known, CardState::New};
    
    json j;
    to_json(j, card);

    EXPECT_EQ(j["id"], "123");
    EXPECT_EQ(j["text_front"], "Front Text");
    EXPECT_EQ(j["text_back"], "Back Text");
    EXPECT_EQ(j["state_Front_to_Back"], "Known");
    EXPECT_EQ(j["state_Back_to_Front"], "New");
}

TEST(JsonMappersTest, FlashcardDeserialization) {
    json j = {
        {"id", "456"},
        {"text_front", "Q"},
        {"text_back", "A"},
        {"state_Front_to_Back", "Mastered"},
        {"state_Back_to_Front", "Known"}
    };

    Flashcard card{};
    from_json(j, card);

    EXPECT_EQ(card.card_id, "456");
    EXPECT_EQ(card.text_front, "Q");
    EXPECT_EQ(card.text_back, "A");
    EXPECT_EQ(card.state_Front_to_Back, CardState::Mastered);
    EXPECT_EQ(card.state_Back_to_Front, CardState::Known);
}

TEST(JsonMappersTest, FlashcardDeserializationMissingFields) {
    json jMissingId = {
        {"text_front", "Q"},
        {"text_back", "A"},
        {"state_Front_to_Back", "Mastered"},
        {"state_Back_to_Front", "Known"}
    };
    
    Flashcard card{};
    // nlohmann::json throws out_of_range when using .at() on a missing key
    EXPECT_THROW(from_json(jMissingId, card), json::out_of_range);
}

// --- FlashcardList Tests ---

TEST(JsonMappersTest, FlashcardListDeserializationErrors) {
    // FlashcardList constructor requires a name, so we use a dummy one
    FlashcardList list("Temp");

    // Edge case: Missing "name"
    json jMissingName = {
        {"cards", json::array()}
    };
    EXPECT_THROW(from_json(jMissingName, list), std::runtime_error);

    // Edge case: Missing "cards" array
    json jMissingCards = {
        {"name", "List"}
    };
    EXPECT_THROW(from_json(jMissingCards, list), std::runtime_error);

    // Edge case: "cards" is not a JSON array
    json jCardsNotArray = {
        {"name", "List"},
        {"cards", "not an array"}
    };
    EXPECT_THROW(from_json(jCardsNotArray, list), std::runtime_error);
    
    // Edge case: card inside array is missing fields
    json jMalformedCard = {
        {"name", "List"},
        {"cards", json::array({ json::object() })} // Empty object instead of full flashcard map
    };
    EXPECT_THROW(from_json(jMalformedCard, list), json::out_of_range);
}