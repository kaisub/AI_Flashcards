#include <gtest/gtest.h>
#include <cctype>
#include "core/CardId.hpp"
#include "storage/JsonMappers.hpp"
#include "storage/JsonSchema.hpp"
#include "core/Flashcard.hpp"
#include "core/FlashcardList.hpp"
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;
using namespace core;

TEST(CardIdTest, GeneratedIdHas12Digits) {
    const std::string id = generateUniqueCardId();
    ASSERT_EQ(id.size(), 12u);
    for (unsigned char c : id) {
        EXPECT_TRUE(std::isdigit(c));
    }
}

// --- CardState Tests ---

TEST(JsonMappersTest, CardStateSerialization) {
    json j1, j2, j3, j4;
    to_json(j1, CardState::New);
    to_json(j2, CardState::Known);
    to_json(j3, CardState::Mastered);
    
    // Edge case: invalid enum value should default to "New"
    to_json(j4, static_cast<CardState>(999));

    EXPECT_EQ(j1, json_values::kStateNew);
    EXPECT_EQ(j2, json_values::kStateKnown);
    EXPECT_EQ(j3, json_values::kStateMastered);
    EXPECT_EQ(j4, json_values::kStateNew);
}

TEST(JsonMappersTest, CardStateDeserialization) {
    CardState state = CardState::New;
    
    json j1 = json_values::kStateNew;
    from_json(j1, state);
    EXPECT_EQ(state, CardState::New);

    json j2 = json_values::kStateKnown;
    from_json(j2, state);
    EXPECT_EQ(state, CardState::Known);

    json j3 = json_values::kStateMastered;
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

    EXPECT_EQ(j[json_keys::kId], "123");
    EXPECT_EQ(j[json_keys::kTextFront], "Front Text");
    EXPECT_EQ(j[json_keys::kTextBack], "Back Text");
    EXPECT_EQ(j[json_keys::kStateFrontToBack], json_values::kStateKnown);
    EXPECT_EQ(j[json_keys::kStateBackToFront], json_values::kStateNew);
}

TEST(JsonMappersTest, FlashcardDeserialization) {
    json j = {
        {json_keys::kId, "456"},
        {json_keys::kTextFront, "Q"},
        {json_keys::kTextBack, "A"},
        {json_keys::kStateFrontToBack, json_values::kStateMastered},
        {json_keys::kStateBackToFront, json_values::kStateKnown}
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
        {json_keys::kTextFront, "Q"},
        {json_keys::kTextBack, "A"},
        {json_keys::kStateFrontToBack, json_values::kStateMastered},
        {json_keys::kStateBackToFront, json_values::kStateKnown}
    };
    
    Flashcard card{};
    EXPECT_NO_THROW(from_json(jMissingId, card));
    EXPECT_TRUE(card.card_id.empty());
}

TEST(JsonMappersTest, FlashcardDeserializationMissingTextDefaultsToPlaceholder) {
    json jMissingText = {
        {json_keys::kId, "x3"},
        {json_keys::kStateFrontToBack, json_values::kStateKnown},
        {json_keys::kStateBackToFront, json_values::kStateMastered}
    };

    Flashcard card{};
    ASSERT_NO_THROW(from_json(jMissingText, card));
    EXPECT_EQ(card.text_front, json_values::kMissingTextPlaceholder);
    EXPECT_EQ(card.text_back, json_values::kMissingTextPlaceholder);
}

TEST(JsonMappersTest, FlashcardDeserializationMissingStateDefaultsToNew) {
    json jMissingStates = {
        {json_keys::kId, "x1"},
        {json_keys::kTextFront, "Q"},
        {json_keys::kTextBack, "A"}
    };

    Flashcard card{};
    ASSERT_NO_THROW(from_json(jMissingStates, card));
    EXPECT_EQ(card.state_Front_to_Back, CardState::New);
    EXPECT_EQ(card.state_Back_to_Front, CardState::New);
}

TEST(JsonMappersTest, FlashcardDeserializationMissingSingleStateDefaultsOnlyMissingOne) {
    json jMissingBackState = {
        {json_keys::kId, "x2"},
        {json_keys::kTextFront, "Q"},
        {json_keys::kTextBack, "A"},
        {json_keys::kStateFrontToBack, json_values::kStateKnown}
    };

    Flashcard card{};
    ASSERT_NO_THROW(from_json(jMissingBackState, card));
    EXPECT_EQ(card.state_Front_to_Back, CardState::Known);
    EXPECT_EQ(card.state_Back_to_Front, CardState::New);
}

// --- FlashcardList Tests ---

TEST(JsonMappersTest, FlashcardListDeserializationErrors) {
    // FlashcardList constructor requires a name, so we use a dummy one
    FlashcardList list("Temp");

    // Edge case: Missing "name"
    json jMissingName = {
        {json_keys::kCards, json::array()}
    };
    EXPECT_THROW(from_json(jMissingName, list), std::runtime_error);

    // Edge case: Missing "cards" array
    json jMissingCards = {
        {json_keys::kName, "List"}
    };
    EXPECT_THROW(from_json(jMissingCards, list), std::runtime_error);

    // Edge case: "cards" is not a JSON array
    json jCardsNotArray = {
        {json_keys::kName, "List"},
        {json_keys::kCards, "not an array"}
    };
    EXPECT_THROW(from_json(jCardsNotArray, list), std::runtime_error);
    
    // Edge case: card inside array can be missing fields and should be normalized.
    json jMalformedCard = {
        {json_keys::kName, "List"},
        {json_keys::kCards, json::array({ json::object() })} // Empty object instead of full flashcard map
    };
    ASSERT_NO_THROW(from_json(jMalformedCard, list));
    ASSERT_EQ(list.size(), 1u);
    auto cards = list.getAllCards();
    ASSERT_EQ(cards.size(), 1u);
    EXPECT_FALSE(cards[0]->card_id.empty());
    EXPECT_EQ(cards[0]->text_front, json_values::kMissingTextPlaceholder);
    EXPECT_EQ(cards[0]->text_back, json_values::kMissingTextPlaceholder);
    EXPECT_EQ(cards[0]->state_Front_to_Back, CardState::New);
    EXPECT_EQ(cards[0]->state_Back_to_Front, CardState::New);
}

TEST(JsonMappersTest, FlashcardListDeserializationGeneratesIdsForMissingId) {
    FlashcardList list("Temp");
    json j = {
        {json_keys::kName, "LegacyList"},
        {json_keys::kCards, json::array({
            {
                {json_keys::kTextFront, "Q1"},
                {json_keys::kTextBack, "A1"},
                {json_keys::kStateFrontToBack, json_values::kStateNew},
                {json_keys::kStateBackToFront, json_values::kStateKnown}
            },
            {
                {json_keys::kId, "ok_id"},
                {json_keys::kTextFront, "Q2"},
                {json_keys::kTextBack, "A2"},
                {json_keys::kStateFrontToBack, json_values::kStateKnown},
                {json_keys::kStateBackToFront, json_values::kStateMastered}
            }
        })}
    };

    ASSERT_NO_THROW(from_json(j, list));
    ASSERT_EQ(list.size(), 2u);

    auto cards = list.getAllCards();
    ASSERT_EQ(cards.size(), 2u);
    EXPECT_FALSE(cards[0]->card_id.empty());
    EXPECT_EQ(cards[1]->card_id, "ok_id");
}

TEST(JsonMappersTest, FlashcardListDeserializationFixesDuplicateIds) {
    FlashcardList list("Temp");
    json j = {
        {json_keys::kName, "DupList"},
        {json_keys::kCards, json::array({
            {
                {json_keys::kId, "dup"},
                {json_keys::kTextFront, "A"},
                {json_keys::kTextBack, "B"},
                {json_keys::kStateFrontToBack, json_values::kStateNew},
                {json_keys::kStateBackToFront, json_values::kStateNew}
            },
            {
                {json_keys::kId, "dup"},
                {json_keys::kTextFront, "C"},
                {json_keys::kTextBack, "D"},
                {json_keys::kStateFrontToBack, json_values::kStateKnown},
                {json_keys::kStateBackToFront, json_values::kStateMastered}
            }
        })}
    };

    ASSERT_NO_THROW(from_json(j, list));
    ASSERT_EQ(list.size(), 2u);
    auto cards = list.getAllCards();
    ASSERT_EQ(cards.size(), 2u);
    EXPECT_NE(cards[0]->card_id, cards[1]->card_id);
}