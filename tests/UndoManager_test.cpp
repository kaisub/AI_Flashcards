#include <gtest/gtest.h>
#include "core/UndoManager.hpp"
#include "core/Flashcard.hpp"
#include <memory>

using namespace core;

TEST(UndoManagerTest, InitialStateIsEmpty) {
    UndoManager undo;
    
    EXPECT_TRUE(undo.isEmpty());
    EXPECT_FALSE(undo.popRecord().has_value());
}

TEST(UndoManagerTest, PushAndPopRecord) {
    UndoManager undo;
    auto card = std::make_shared<Flashcard>();
    card->id = "test-1";

    HistoryRecord record{
        "test-1",
        card,
        CardState::New,
        CardState::Known,
        TranslationDirection::Front_to_Back,
        CardState::New // pushedToQueueState
    };

    undo.pushRecord(record);
    EXPECT_FALSE(undo.isEmpty());

    auto popped = undo.popRecord();
    ASSERT_TRUE(popped.has_value());
    
    EXPECT_EQ(popped->cardId, "test-1");
    EXPECT_EQ(popped->cardPtr, card);
    EXPECT_EQ(popped->previousFrontState, CardState::New);
    EXPECT_EQ(popped->previousBackState, CardState::Known);
    EXPECT_EQ(popped->askedDirection, TranslationDirection::Front_to_Back);
    EXPECT_EQ(popped->pushedToQueueState, CardState::New);

    EXPECT_TRUE(undo.isEmpty());
}

TEST(UndoManagerTest, ClearRemovesAllRecords) {
    UndoManager undo;
    undo.pushRecord(HistoryRecord{"test-1", nullptr, CardState::New, CardState::Known, TranslationDirection::Mixed, CardState::Known});
    undo.pushRecord(HistoryRecord{"test-2", nullptr, CardState::Known, CardState::Mastered, TranslationDirection::Front_to_Back, CardState::Mastered});
    EXPECT_FALSE(undo.isEmpty());

    undo.clear();

    EXPECT_TRUE(undo.isEmpty());
    EXPECT_FALSE(undo.popRecord().has_value());
}

TEST(UndoManagerTest, MultipleRecordsLIFO) {
    UndoManager undo;
    undo.pushRecord(HistoryRecord{"1", nullptr, CardState::New, CardState::New, TranslationDirection::Mixed, CardState::New});
    undo.pushRecord(HistoryRecord{"2", nullptr, CardState::New, CardState::New, TranslationDirection::Mixed, CardState::New});

    // Pop should return the last pushed item (LIFO)
    auto pop1 = undo.popRecord();
    ASSERT_TRUE(pop1.has_value());
    EXPECT_EQ(pop1->cardId, "2");

    auto pop2 = undo.popRecord();
    ASSERT_TRUE(pop2.has_value());
    EXPECT_EQ(pop2->cardId, "1");

    EXPECT_TRUE(undo.isEmpty());
}