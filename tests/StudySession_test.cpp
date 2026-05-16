#include <gtest/gtest.h>

#include "core/StudySession.hpp"
#include "core/Flashcard.hpp"

#include <vector>
#include <memory>
#include <map>     // For frequency counting
#include <cmath>   // For std::abs
#include <algorithm> // For std::shuffle
#include <random>    // For std::mt19937 and std::random_device

#include "utils/TestFactory.hpp"

using namespace core;

// Helper to create a shared_ptr Flashcard
using test_utils::createTestFlashcard;

TEST(StudySessionTest, SessionConfig_ResetToDefaults) {
    SessionConfig config;
    config.type = SessionType::Focused;
    config.focusedTargetState = CardState::Mastered;
    config.direction = TranslationDirection::Front_to_Back;
    config.order = SessionOrder::Queue;
    config.weightNew = 10;
    config.weightKnown = 10;
    config.weightMastered = 10;

    config.resetToDefaults();

    EXPECT_EQ(config.type, SessionType::Standard);
    EXPECT_FALSE(config.focusedTargetState.has_value());
    EXPECT_EQ(config.direction, TranslationDirection::Mixed);
    EXPECT_EQ(config.order, SessionOrder::Random);
    EXPECT_EQ(config.weightNew, 70);
    EXPECT_EQ(config.weightKnown, 23);
    EXPECT_EQ(config.weightMastered, 7);
}

TEST(StudySessionTest, StandardMode_RespectsDrawProbabilities) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    // Create a deck with 70 New, 23 Known, 7 Mastered cards
    for (int i = 0; i < 70; ++i) deck.push_back(createTestFlashcard("new" + std::to_string(i), "N", "N", CardState::New, CardState::New));
    for (int i = 0; i < 23; ++i) deck.push_back(createTestFlashcard("known" + std::to_string(i), "K", "K", CardState::Known, CardState::Known));
    for (int i = 0; i < 7; ++i) deck.push_back(createTestFlashcard("mastered" + std::to_string(i), "M", "M", CardState::Mastered, CardState::Mastered));

    // Shuffle the initial deck for varied input (StudySession will shuffle internally too)
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck.begin(), deck.end(), g);

    SessionConfig config;
    config.type = SessionType::Standard;
    config.weightNew = 70;
    config.weightKnown = 23;
    config.weightMastered = 7;
    config.direction = TranslationDirection::Front_to_Back; // Fix direction for consistent state checks

    StudySession session(deck, config);

    const int iterations = 5000; // Increased iterations for better probability approximation
    std::map<CardState, int> drawCounts;
    drawCounts[CardState::New] = 0;
    drawCounts[CardState::Known] = 0;
    drawCounts[CardState::Mastered] = 0;

    for (int i = 0; i < iterations; ++i) {
        std::optional<ReviewItem> item = session.getNextItem();
        ASSERT_TRUE(item.has_value()); // Should always return an item until all are exhausted

        CardState originalState = CardState::New;
        // Determine the originating bucket from immutable ID prefix.
        // Card state mutates after submitAnswer, so using state here biases counts.
        if (item->card->id.rfind("new", 0) == 0) {
            drawCounts[CardState::New]++;
            originalState = CardState::New;
        } else if (item->card->id.rfind("known", 0) == 0) {
            drawCounts[CardState::Known]++;
            originalState = CardState::Known;
        } else if (item->card->id.rfind("mastered", 0) == 0) {
            drawCounts[CardState::Mastered]++;
            originalState = CardState::Mastered;
        }

        // Submit answer with its original state to keep it in its original queue.
        // This prevents queues from emptying, which would alter the active probability weights.
        session.submitAnswer(item.value(), originalState);
    }

    // Calculate expected counts
    double totalWeight = config.weightNew + config.weightKnown + config.weightMastered;
    double expectedNew = (static_cast<double>(config.weightNew) / totalWeight) * iterations;
    double expectedKnown = (static_cast<double>(config.weightKnown) / totalWeight) * iterations;
    double expectedMastered = (static_cast<double>(config.weightMastered) / totalWeight) * iterations;

    // Calculate margins based on binomial distribution standard deviation bounds (>= 5 sigma for CI stability).
    // A flat 10% margin makes smaller probabilities (like Mastered at 7%) highly prone to natural
    // variance failures (1.9 sigma = ~5% random failure rate).
    double epsilonNew = expectedNew * 0.10;        // ~10.8 sigma
    double epsilonKnown = expectedKnown * 0.15;    // ~5.8 sigma
    double epsilonMastered = expectedMastered * 0.30; // ~5.8 sigma

    EXPECT_NEAR(static_cast<double>(drawCounts[CardState::New]), expectedNew, epsilonNew);
    EXPECT_NEAR(static_cast<double>(drawCounts[CardState::Known]), expectedKnown, epsilonKnown);
    EXPECT_NEAR(static_cast<double>(drawCounts[CardState::Mastered]), expectedMastered, epsilonMastered);

    // Small sanity check for total draws
    EXPECT_EQ(drawCounts[CardState::New] + drawCounts[CardState::Known] + drawCounts[CardState::Mastered], iterations);
}

TEST(StudySessionTest, StandardMode_HandlesEmptyQueues) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    deck.push_back(createTestFlashcard("new1", "N1", "N1", CardState::New, CardState::New));
    deck.push_back(createTestFlashcard("new2", "N2", "N2", CardState::New, CardState::New));
    deck.push_back(createTestFlashcard("known1", "K1", "K1", CardState::Known, CardState::Known));

    SessionConfig config;
    config.type = SessionType::Standard;
    config.weightNew = 100;
    config.weightKnown = 1; // Very low weight
    config.weightMastered = 0; // Zero weight, empty queue

    StudySession session(deck, config);

    // Initial state
    EXPECT_EQ(session.getQueueSize(CardState::New), 2u);
    EXPECT_EQ(session.getQueueSize(CardState::Known), 1u);
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 0u);

    // Exhaust the 'New' queue
    std::optional<ReviewItem> item1 = session.getNextItem(); // Should be N1 or N2
    ASSERT_TRUE(item1.has_value());
    EXPECT_EQ(item1->card->id.substr(0,3), "new");
    session.submitAnswer(item1.value(), CardState::Mastered); // Move to Mastered, which is empty, but now has it

    std::optional<ReviewItem> item2 = session.getNextItem(); // Should be N1 or N2 (the other one)
    ASSERT_TRUE(item2.has_value());
    EXPECT_EQ(item2->card->id.substr(0,3), "new");
    session.submitAnswer(item2.value(), CardState::Mastered);

    EXPECT_EQ(session.getQueueSize(CardState::New), 0u);
    EXPECT_EQ(session.getQueueSize(CardState::Known), 1u);
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 2u); // N1, N2 are now here

    // Next item should come from 'Known' or 'Mastered' as 'New' is empty
    // Since Mastered queue has a weight of 0, it should primarily draw from Known if it has a non-zero weight.
    std::optional<ReviewItem> item3 = session.getNextItem();
    ASSERT_TRUE(item3.has_value());
    // Due to the very low weight of 'Known' and 'Mastered' now having 2 cards with weight 0,
    // the dynamic weights will normalize. The K1 card from 'Known' should be drawn.
    EXPECT_EQ(item3->card->id, "known1");
    session.submitAnswer(item3.value(), CardState::Mastered); // K1 now also in Mastered

    EXPECT_EQ(session.getQueueSize(CardState::New), 0u);
    EXPECT_EQ(session.getQueueSize(CardState::Known), 0u);
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 3u); // N1, N2, K1 are here

    // All original cards now in Mastered queue (which has 0 weight).
    // The StandardMode_RespectsDrawProbabilities test showed that 0-weighted queues are not chosen.
    // If all *non-zero* weighted queues are empty, it should try other queues.
    // In current implementation, if queue is empty, it won't be in weightedQueues.
    // So it effectively normalizes.

    // The next draw should come from the 'Mastered' queue, even if its weight is 0 initially,
    // because it's the only non-empty queue left.
    // This is implicitly handled by `weightedQueues.push_back` which only considers non-empty queues.
    std::optional<ReviewItem> item4 = session.getNextItem();
    ASSERT_TRUE(item4.has_value());
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 2u); // One card drawn
    session.submitAnswer(item4.value(), CardState::Mastered); // Re-queue to keep total card count stable

    // Exhaust all cards
    item4 = session.getNextItem();
    ASSERT_TRUE(item4.has_value());
    session.submitAnswer(item4.value(), CardState::Mastered);

    item4 = session.getNextItem();
    ASSERT_TRUE(item4.has_value());
    session.submitAnswer(item4.value(), CardState::Mastered);

    EXPECT_EQ(session.getQueueSize(CardState::New), 0u);
    EXPECT_EQ(session.getQueueSize(CardState::Known), 0u);
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 3u); // All cards submitted back

    // Now if we draw again, it should eventually be nullopt if no more unique cards are generated.
    // But since submitAnswer puts them back, the session technically never ends unless we remove.
    // The test case requirement is "ensure the session does not crash or infinitely loop if one queue is empty"
    // which it handles by only considering non-empty queues for `weightedQueues`.
    // And "instead normalizes weights" which is implicitly done by `std::discrete_distribution`.
}

TEST(StudySessionTest, StandardMode_ZeroWeights_FallsBackToUniform) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    deck.push_back(createTestFlashcard("new1", "N1", "N1", CardState::New, CardState::New));
    deck.push_back(createTestFlashcard("known1", "K1", "K1", CardState::Known, CardState::Known));
    deck.push_back(createTestFlashcard("mastered1", "M1", "M1", CardState::Mastered, CardState::Mastered));

    SessionConfig config;
    config.type = SessionType::Standard;
    config.weightNew = 0;
    config.weightKnown = 0;
    config.weightMastered = 0;

    StudySession session(deck, config);

    // Even with all 0 weights, it shouldn't crash and should safely exhaust all 3 cards eventually
    ASSERT_TRUE(session.getNextItem().has_value());
    ASSERT_TRUE(session.getNextItem().has_value());
    ASSERT_TRUE(session.getNextItem().has_value());
    EXPECT_FALSE(session.getNextItem().has_value()); // Exhausted
}

TEST(StudySessionTest, FocusedMode_FiniteExhaustion) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    deck.push_back(createTestFlashcard("new1", "N1", "N1", CardState::New, CardState::New));
    deck.push_back(createTestFlashcard("new2", "N2", "N2", CardState::New, CardState::New));
    deck.push_back(createTestFlashcard("known1", "K1", "K1", CardState::Known, CardState::Known));
    deck.push_back(createTestFlashcard("mastered1", "M1", "M1", CardState::Mastered, CardState::Mastered));

    SessionConfig config;
    config.type = SessionType::Focused;
    config.focusedTargetState = CardState::New;
    config.direction = TranslationDirection::Front_to_Back;

    StudySession session(deck, config);

    // Initial state
    EXPECT_EQ(session.getQueueSize(CardState::New), 2u);
    EXPECT_EQ(session.getQueueSize(CardState::Known), 1u);
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 1u);
    EXPECT_EQ(session.getTotalRemainingInFocusedMode(), 2u);

    // Draw first new card
    std::optional<ReviewItem> item1 = session.getNextItem();
    ASSERT_TRUE(item1.has_value());
    EXPECT_EQ(item1->card->id.substr(0,3), "new");
    EXPECT_EQ(session.getQueueSize(CardState::New), 1u);
    EXPECT_EQ(session.getTotalRemainingInFocusedMode(), 1u);
    session.submitAnswer(item1.value(), CardState::Known); // Move out of 'New' queue

    // Draw second new card
    std::optional<ReviewItem> item2 = session.getNextItem();
    ASSERT_TRUE(item2.has_value());
    EXPECT_EQ(item2->card->id.substr(0,3), "new");
    EXPECT_EQ(session.getQueueSize(CardState::New), 0u);
    EXPECT_EQ(session.getTotalRemainingInFocusedMode(), 0u);
    session.submitAnswer(item2.value(), CardState::Known); // Move out of 'New' queue

    // Now 'New' queue should be empty. getNextItem should return nullopt.
    std::optional<ReviewItem> item3 = session.getNextItem();
    EXPECT_FALSE(item3.has_value());

    EXPECT_EQ(session.getQueueSize(CardState::New), 0u);
    EXPECT_EQ(session.getQueueSize(CardState::Known), 3u); // Two 'new' cards moved here + original 'known'
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 1u);
}

TEST(StudySessionTest, FocusedMode_MissingTargetState_ReturnsNullopt) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    deck.push_back(createTestFlashcard("new1", "N1", "N1", CardState::New, CardState::New));

    SessionConfig config;
    config.type = SessionType::Focused;
    config.focusedTargetState = std::nullopt; // Missing

    StudySession session(deck, config);

    // Should immediately return nullopt without attempting to crash
    EXPECT_FALSE(session.getNextItem().has_value());
}

TEST(StudySessionTest, SubmitAnswer_UpdatesStateAndQueue) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    auto card1 = createTestFlashcard("c1", "F1", "B1", CardState::New, CardState::New);
    auto card2 = createTestFlashcard("c2", "F2", "B2", CardState::Known, CardState::New); // Card2 is Known F->B
    deck.push_back(card1);
    deck.push_back(card2);

    SessionConfig config;
    config.type = SessionType::Standard;
    config.direction = TranslationDirection::Front_to_Back; // Fixed direction for predictability
    config.weightNew = 100;
    config.weightKnown = 0;
    config.weightMastered = 0;

    StudySession session(deck, config);

    // Initial queue sizes
    EXPECT_EQ(session.getQueueSize(CardState::New), 1u); // Only card1 is New F->B
    EXPECT_EQ(session.getQueueSize(CardState::Known), 1u); // Only card2 is Known F->B
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 0u);

    // Draw a card. With weightNew=100 and others=0, we are guaranteed to draw c1.
    std::optional<ReviewItem> item1_opt = session.getNextItem();
    ASSERT_TRUE(item1_opt.has_value());
    ReviewItem item1 = item1_opt.value();

    EXPECT_EQ(item1.card->id, "c1");
    EXPECT_EQ(session.getQueueSize(CardState::New), 0u); // card1 removed from New
    EXPECT_EQ(card1->state_Front_to_Back, CardState::New); // State before submission

    // Submit answer, move to Known
    session.submitAnswer(item1, CardState::Known);
    EXPECT_EQ(card1->state_Front_to_Back, CardState::Known); // State updated
    EXPECT_EQ(session.getQueueSize(CardState::Known), 2u); // card1 added to Known queue

    // Draw next card. Only Known queue has cards now (c2, then c1).
    std::optional<ReviewItem> item2_opt = session.getNextItem();
    ASSERT_TRUE(item2_opt.has_value());
    ReviewItem item2 = item2_opt.value();

    // Verify it's the other card (c2 is at the front of the Known queue)
    EXPECT_EQ(item2.card->id, "c2");
    EXPECT_EQ(card2->state_Front_to_Back, CardState::Known); // Still Known from initial state
}


TEST(StudySessionTest, UndoAction_RestoresPreviousState) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    auto cardA = createTestFlashcard("A", "F_A", "B_A", CardState::New, CardState::New);
    auto cardB = createTestFlashcard("B", "F_B", "B_B", CardState::New, CardState::New);
    deck.push_back(cardA);
    deck.push_back(cardB);

    SessionConfig config;
    config.type = SessionType::Standard;
    config.direction = TranslationDirection::Front_to_Back; // Fix direction
    StudySession session(deck, config);

    // Initial state
    EXPECT_EQ(session.getQueueSize(CardState::New), 2u);
    EXPECT_EQ(session.getQueueSize(CardState::Known), 0u);
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 0u);

    // 1. Draw card A, submit as Known
    std::optional<ReviewItem> itemA_opt = session.getNextItem();
    ASSERT_TRUE(itemA_opt.has_value());
    ReviewItem itemA = itemA_opt.value();
    ASSERT_EQ(itemA.card->state_Front_to_Back, CardState::New); // Original state

    session.submitAnswer(itemA, CardState::Known);
    EXPECT_EQ(itemA.card->state_Front_to_Back, CardState::Known); // New state
    EXPECT_EQ(session.getQueueSize(CardState::New), 1u); // One card left in new (B)
    EXPECT_EQ(session.getQueueSize(CardState::Known), 1u); // A moved to known

    // Make the next draw deterministic: only draw from New.
    SessionConfig cfgNewOnly = config;
    cfgNewOnly.weightNew = 1;
    cfgNewOnly.weightKnown = 0;
    cfgNewOnly.weightMastered = 0;
    session.updateConfig(cfgNewOnly);

    // 2. Draw card B, submit as Mastered
    std::optional<ReviewItem> itemB_opt = session.getNextItem();
    ASSERT_TRUE(itemB_opt.has_value());
    ReviewItem itemB = itemB_opt.value();
    ASSERT_EQ(itemB.card->state_Front_to_Back, CardState::New); // Original state

    session.submitAnswer(itemB, CardState::Mastered);
    EXPECT_EQ(itemB.card->state_Front_to_Back, CardState::Mastered); // New state
    EXPECT_EQ(session.getQueueSize(CardState::New), 0u); // No cards left in new
    EXPECT_EQ(session.getQueueSize(CardState::Known), 1u); // A is still known
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 1u); // B moved to mastered

    // Undo last action (card B)
    EXPECT_TRUE(session.undoLastAction());
    EXPECT_EQ(itemB.card->state_Front_to_Back, CardState::New); // Card B state reverted to New
    EXPECT_EQ(session.getQueueSize(CardState::New), 1u); // B moved back to New queue
    EXPECT_EQ(session.getQueueSize(CardState::Known), 1u); // A is still known
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 0u); // B removed from mastered

    // Undo another action (card A)
    EXPECT_TRUE(session.undoLastAction());
    EXPECT_EQ(itemA.card->state_Front_to_Back, CardState::New); // Card A state reverted to New
    EXPECT_EQ(session.getQueueSize(CardState::New), 2u); // A moved back to New queue
    EXPECT_EQ(session.getQueueSize(CardState::Known), 0u); // A removed from known
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 0u);

    // Try undoing when history is empty
    EXPECT_FALSE(session.undoLastAction());
    EXPECT_EQ(session.getQueueSize(CardState::New), 2u); // No change
}

TEST(StudySessionTest, RemoveCard_SafelyEjectsCard) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    auto card1 = createTestFlashcard("c1", "F1", "B1", CardState::New, CardState::New);
    auto card2 = createTestFlashcard("c2", "F2", "B2", CardState::Known, CardState::Known);
    auto card3 = createTestFlashcard("c3", "F3", "B3", CardState::Mastered, CardState::Mastered);
    deck.push_back(card1);
    deck.push_back(card2);
    deck.push_back(card3);

    SessionConfig config;
    config.type = SessionType::Standard;
    config.direction = TranslationDirection::Front_to_Back;
    StudySession session(deck, config);

    // Initial state
    EXPECT_EQ(session.getQueueSize(CardState::New), 1u);
    EXPECT_EQ(session.getQueueSize(CardState::Known), 1u);
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 1u);

    // Remove card2 (from Known queue)
    EXPECT_TRUE(session.removeCardFromSession("c2"));
    EXPECT_EQ(session.getQueueSize(CardState::New), 1u);
    EXPECT_EQ(session.getQueueSize(CardState::Known), 0u); // Card2 removed
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 1u);

    // Try removing a non-existent card
    EXPECT_FALSE(session.removeCardFromSession("nonexistent"));

    // Ensure removed card is never returned
    std::optional<ReviewItem> item;
    int drawnCount = 0;
    while (drawnCount < 10) { // Limit draws to avoid infinite loop
        item = session.getNextItem();
        if (!item.has_value()) {
            break;
        }
        EXPECT_NE(item->card->id, "c2"); // Should never return card2
        session.submitAnswer(item.value(), CardState::Mastered); // Keep submitting to keep session active
        drawnCount++;
    }

    // After exhausting remaining cards (c1 and c3), all cards should have been drawn once and then re-queued
    // If we draw for a long time, c2 should still not appear.
    // Let's ensure c1 and c3 are still present
    EXPECT_EQ(session.getQueueSize(CardState::New) + session.getQueueSize(CardState::Known) + session.getQueueSize(CardState::Mastered), 2u);
}

TEST(StudySessionTest, MixedDirectionHandling) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    auto card1 = createTestFlashcard("c1", "Front", "Back", CardState::New, CardState::Known);
    deck.push_back(card1);

    SessionConfig config;
    config.type = SessionType::Standard;
    config.direction = TranslationDirection::Mixed;
    StudySession session(deck, config);

    // Initial queues: c1 (New F->B, Known B->F)
    // If getRandomDirection picks F->B, it goes to New queue.
    // If getRandomDirection picks B->F, it goes to Known queue.
    // We can't predict, so check sizes relative to total
    size_t initialNew = session.getQueueSize(CardState::New);
    size_t initialKnown = session.getQueueSize(CardState::Known);
    EXPECT_EQ(initialNew + initialKnown, 1u);
    EXPECT_EQ(session.getQueueSize(CardState::Mastered), 0u);

    // Draw and submit, see where it ends up. It should preserve state for the other direction.
    std::optional<ReviewItem> item_opt = session.getNextItem();
    ASSERT_TRUE(item_opt.has_value());
    ReviewItem item = item_opt.value();

    CardState oldFrontState = item.card->state_Front_to_Back;
    CardState oldBackState = item.card->state_Back_to_Front;
    TranslationDirection askedDir = item.askedDirection;

    // Submit as Mastered
    session.submitAnswer(item, CardState::Mastered);

    // Check states: only the asked direction's state should change
    if (askedDir == TranslationDirection::Front_to_Back) {
        EXPECT_EQ(item.card->state_Front_to_Back, CardState::Mastered);
        EXPECT_EQ(item.card->state_Back_to_Front, oldBackState); // Other direction unchanged
    } else { // Back_to_Front
        EXPECT_EQ(item.card->state_Back_to_Front, CardState::Mastered);
        EXPECT_EQ(item.card->state_Front_to_Back, oldFrontState); // Other direction unchanged
    }

    // Now undo, should revert the specific state change
    EXPECT_TRUE(session.undoLastAction());
    if (askedDir == TranslationDirection::Front_to_Back) {
        EXPECT_EQ(item.card->state_Front_to_Back, oldFrontState);
        EXPECT_EQ(item.card->state_Back_to_Front, oldBackState);
    } else {
        EXPECT_EQ(item.card->state_Back_to_Front, oldBackState);
        EXPECT_EQ(item.card->state_Front_to_Back, oldFrontState);
    }
}

TEST(StudySessionTest, ConfigUpdate_ChangesDirectionMidSession) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    auto card = createTestFlashcard("c1", "Front", "Back", CardState::New, CardState::New);
    deck.push_back(card);

    SessionConfig config;
    config.type = SessionType::Standard;
    config.direction = TranslationDirection::Front_to_Back;
    StudySession session(deck, config);

    // Draw the item. It is guaranteed to be asked Front_to_Back.
    std::optional<ReviewItem> item_opt = session.getNextItem();
    ASSERT_TRUE(item_opt.has_value());
    EXPECT_EQ(item_opt->askedDirection, TranslationDirection::Front_to_Back);

    // Now update config direction to Back_to_Front
    SessionConfig newConfig = config;
    newConfig.direction = TranslationDirection::Back_to_Front;
    session.updateConfig(newConfig);

    // Submit the answer. Because it was drawn F->B, it should evaluate F->B,
    // correctly ignoring the new config during submission.
    session.submitAnswer(item_opt.value(), CardState::Mastered);

    EXPECT_EQ(card->state_Front_to_Back, CardState::Mastered);
    EXPECT_EQ(card->state_Back_to_Front, CardState::New); // Unchanged!

    // But the next time we draw it, it should be asked Back_to_Front due to the new config queueing strategy.
    std::optional<ReviewItem> item_opt2 = session.getNextItem();
    ASSERT_TRUE(item_opt2.has_value());
    EXPECT_EQ(item_opt2->askedDirection, TranslationDirection::Back_to_Front);
}

TEST(StudySessionTest, UndoAfterRemoveCard_SkipsRemovedCard) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    deck.push_back(createTestFlashcard("A", "F_A", "B_A", CardState::New, CardState::New));
    deck.push_back(createTestFlashcard("B", "F_B", "B_B", CardState::Known, CardState::Known));

    SessionConfig config;
    config.type = SessionType::Standard;
    config.weightNew = 100;
    config.weightKnown = 0; // Draw A first
    config.weightMastered = 0;
    StudySession session(deck, config);

    auto itemA = session.getNextItem();
    session.submitAnswer(itemA.value(), CardState::Mastered);

    config.weightNew = 0;
    config.weightKnown = 100; // Draw B next
    session.updateConfig(config);

    auto itemB = session.getNextItem();
    session.submitAnswer(itemB.value(), CardState::Mastered);

    EXPECT_TRUE(session.removeCardFromSession("B"));

    // First undo targets 'B'. Since 'B' was removed, undo safely pops history and returns false without crashing.
    EXPECT_FALSE(session.undoLastAction());

    // Next undo targets 'A', which still exists and should succeed.
    EXPECT_TRUE(session.undoLastAction());
    EXPECT_EQ(deck[0]->state_Front_to_Back, CardState::New);
}

TEST(StudySessionTest, Undo_MakesLastSubmittedCardNext_WhenCardsWereNew) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    deck.push_back(createTestFlashcard("1", "F1", "B1", CardState::New, CardState::New));
    deck.push_back(createTestFlashcard("2", "F2", "B2", CardState::New, CardState::New));
    deck.push_back(createTestFlashcard("3", "F3", "B3", CardState::New, CardState::New));
    deck.push_back(createTestFlashcard("4", "F4", "B4", CardState::New, CardState::New));

    SessionConfig config;
    config.type = SessionType::Standard;
    config.direction = TranslationDirection::Front_to_Back;
    config.weightNew = 100;
    config.weightKnown = 0;
    config.weightMastered = 0;

    StudySession session(deck, config, 123u);

    std::string lastSubmittedId;
    for (int i = 0; i < 4; ++i) {
        auto item = session.getNextItem();
        ASSERT_TRUE(item.has_value());
        lastSubmittedId = item->card->id;
        session.submitAnswer(item.value(), CardState::Mastered);
    }

    ASSERT_TRUE(session.undoLastAction());
    auto replay = session.getNextItem();
    ASSERT_TRUE(replay.has_value());
    EXPECT_EQ(replay->card->id, lastSubmittedId);
}

TEST(StudySessionTest, Undo_MakesLastSubmittedCardNext_WhenCardsWereMastered) {
    std::vector<std::shared_ptr<Flashcard>> deck;
    deck.push_back(createTestFlashcard("1", "F1", "B1", CardState::Mastered, CardState::Mastered));
    deck.push_back(createTestFlashcard("2", "F2", "B2", CardState::Mastered, CardState::Mastered));
    deck.push_back(createTestFlashcard("3", "F3", "B3", CardState::Mastered, CardState::Mastered));
    deck.push_back(createTestFlashcard("4", "F4", "B4", CardState::Mastered, CardState::Mastered));

    SessionConfig config;
    config.type = SessionType::Standard;
    config.direction = TranslationDirection::Front_to_Back;
    config.weightNew = 0;
    config.weightKnown = 0;
    config.weightMastered = 100;

    StudySession session(deck, config, 456u);

    std::string lastSubmittedId;
    for (int i = 0; i < 4; ++i) {
        auto item = session.getNextItem();
        ASSERT_TRUE(item.has_value());
        lastSubmittedId = item->card->id;
        session.submitAnswer(item.value(), CardState::Mastered);
    }

    ASSERT_TRUE(session.undoLastAction());
    auto replay = session.getNextItem();
    ASSERT_TRUE(replay.has_value());
    EXPECT_EQ(replay->card->id, lastSubmittedId);
}