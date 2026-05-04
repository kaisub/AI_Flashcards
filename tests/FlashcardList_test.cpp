#include <gtest/gtest.h>

#include "core/FlashcardList.hpp"
#include "core/Flashcard.hpp" // For creating Flashcard objects
#include "utils/TestFactory.hpp"

#include <memory>
#include <vector>
#include <string>

using namespace core;

TEST(FlashcardListTest, AddAndRetrieveCard) {
    FlashcardList list("MyList");
    auto card1 = test_utils::createTestFlashcard("1", "Hello", "World");

    // Add card
    EXPECT_TRUE(list.addCard(card1));
    EXPECT_EQ(list.size(), 1u);

    // Retrieve card
    std::shared_ptr<Flashcard> retrievedCard = list.getCard("1");
    ASSERT_NE(retrievedCard, nullptr);
    EXPECT_EQ(retrievedCard->text_front, "Hello");
    EXPECT_EQ(retrievedCard->text_back, "World");
    EXPECT_EQ(retrievedCard->id, "1");

    // Verify O(1) retrieval by checking pointer identity
    EXPECT_EQ(retrievedCard, card1);

    // Try adding a duplicate ID
    auto card2 = test_utils::createTestFlashcard("1", "Hola", "Mundo"); // Same ID
    EXPECT_FALSE(list.addCard(card2));
    EXPECT_EQ(list.size(), 1u); // Size should remain 1

    // Ensure the original card is still there, not overwritten
    retrievedCard = list.getCard("1");
    ASSERT_NE(retrievedCard, nullptr);
    EXPECT_EQ(retrievedCard->text_front, "Hello");
    EXPECT_EQ(retrievedCard->text_back, "World");

    // Add another card with a different ID
    auto card3 = test_utils::createTestFlashcard("3", "Bonjour", "Le Monde");
    EXPECT_TRUE(list.addCard(card3));
    EXPECT_EQ(list.size(), 2u);
}

TEST(FlashcardListTest, UpdateCardData) {
    FlashcardList list("MyList");
    auto card = test_utils::createTestFlashcard("101", "Old Front", "Old Back");
    list.addCard(card);

    // Update existing card
    EXPECT_TRUE(list.updateCard("101", "New Front", "New Back"));

    std::shared_ptr<Flashcard> updatedCard = list.getCard("101");
    ASSERT_NE(updatedCard, nullptr);
    EXPECT_EQ(updatedCard->text_front, "New Front");
    EXPECT_EQ(updatedCard->text_back, "New Back");
    EXPECT_EQ(updatedCard->id, "101");

    // Verify the original shared_ptr also reflects the update
    EXPECT_EQ(card->text_front, "New Front");
    EXPECT_EQ(card->text_back, "New Back");

    // Try updating a non-existent card
    EXPECT_FALSE(list.updateCard("999", "Non Existent Front", "Non Existent Back"));
}

TEST(FlashcardListTest, BulkImportAndRemove) {
    FlashcardList list1("List1");
    FlashcardList list2("List2");

    std::vector<std::shared_ptr<Flashcard>> cardsToImport;
    cardsToImport.push_back(test_utils::createTestFlashcard("A1", "Apple", "Jabłko"));
    cardsToImport.push_back(test_utils::createTestFlashcard("B2", "Banana", "Banan"));
    cardsToImport.push_back(test_utils::createTestFlashcard("C3", "Cherry", "Czereśnia"));
    cardsToImport.push_back(test_utils::createTestFlashcard("D4", "Date", "Daktyl"));

    // Bulk import a vector of cards to List1
    size_t addedCount1 = list1.importCardsFrom(cardsToImport);
    EXPECT_EQ(addedCount1, 4u);
    EXPECT_EQ(list1.size(), 4u);
    EXPECT_NE(list1.getCard("A1"), nullptr);
    EXPECT_NE(list1.getCard("D4"), nullptr);

    // Attempt to import some cards again (duplicates should be rejected)
    cardsToImport.push_back(test_utils::createTestFlashcard("A1", "Duplicate Apple", "Duplicate Jabłko")); // Duplicate ID
    cardsToImport.push_back(test_utils::createTestFlashcard("E5", "Elderberry", "Bez"));
    size_t addedCount1_reimport = list1.importCardsFrom(cardsToImport);
    EXPECT_EQ(addedCount1_reimport, 1u); // Only E5 should be added
    EXPECT_EQ(list1.size(), 5u);

    // Bulk import from List1 to List2
    size_t addedCount2 = list2.importCardsFrom(list1);
    EXPECT_EQ(addedCount2, 5u);
    EXPECT_EQ(list2.size(), 5u);
    EXPECT_NE(list2.getCard("C3"), nullptr);
    EXPECT_EQ(list2.getCard("A1"), list1.getCard("A1")); // Verify shared ownership

    // Remove multiple cards from List1
    std::vector<std::string> idsToRemove = {"B2", "D4", "F6"}; // F6 does not exist
    size_t removedCount = list1.removeCards(idsToRemove);
    EXPECT_EQ(removedCount, 2u); // B2 and D4 should be removed
    EXPECT_EQ(list1.size(), 3u);
    EXPECT_EQ(list1.getCard("B2"), nullptr);
    EXPECT_EQ(list1.getCard("D4"), nullptr);
    EXPECT_NE(list1.getCard("A1"), nullptr);
    EXPECT_NE(list1.getCard("C3"), nullptr);
    EXPECT_NE(list1.getCard("E5"), nullptr);

    // Check List2 is unaffected by removals in List1 (as they are shared_ptrs)
    EXPECT_EQ(list2.size(), 5u);
    EXPECT_NE(list2.getCard("B2"), nullptr);
    EXPECT_NE(list2.getCard("D4"), nullptr);
}

TEST(FlashcardListTest, RemoveCard) {
    FlashcardList list("MyList");
    auto card1 = test_utils::createTestFlashcard("1", "A", "B");
    list.addCard(card1);
    EXPECT_EQ(list.size(), 1u);

    // Remove existing card
    EXPECT_TRUE(list.removeCard("1"));
    EXPECT_EQ(list.size(), 0u);
    EXPECT_EQ(list.getCard("1"), nullptr);

    // Try removing a non-existent card
    EXPECT_FALSE(list.removeCard("1"));
    EXPECT_FALSE(list.removeCard("999"));
}

TEST(FlashcardListTest, ClearList) {
    FlashcardList list("MyList");
    list.addCard(test_utils::createTestFlashcard("1", "A", "B"));
    list.addCard(test_utils::createTestFlashcard("2", "C", "D"));
    EXPECT_EQ(list.size(), 2u);

    list.clear();
    EXPECT_EQ(list.size(), 0u);
    EXPECT_EQ(list.getCard("1"), nullptr);
    EXPECT_EQ(list.getCard("2"), nullptr);
}

TEST(FlashcardListTest, GetAllCards) {
    FlashcardList list("MyList");
    auto card1 = test_utils::createTestFlashcard("1", "A", "B");
    auto card2 = test_utils::createTestFlashcard("2", "C", "D");
    list.addCard(card1);
    list.addCard(card2);

    std::vector<std::shared_ptr<Flashcard>> allCards = list.getAllCards();
    EXPECT_EQ(allCards.size(), 2u);

    bool found1 = false;
    bool found2 = false;
    for (const auto& card : allCards) {
        if (card->id == "1") found1 = true;
        if (card->id == "2") found2 = true;
    }
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);

    // Test with an empty list
    FlashcardList emptyList("Empty");
    std::vector<std::shared_ptr<Flashcard>> emptyCards = emptyList.getAllCards();
    EXPECT_TRUE(emptyCards.empty());
}