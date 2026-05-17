#include <gtest/gtest.h>

#include "core/DeckManager.hpp"
#include "core/Flashcard.hpp"
#include "utils/TestFactory.hpp"

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <filesystem>

using namespace core;

TEST(DeckManagerTest, CreateAndDeleteLists) {
    DeckManager dm(nullptr);

    EXPECT_TRUE(dm.createList("Deck1"));
    EXPECT_NE(dm.getList("Deck1"), nullptr);
    std::vector<std::string> names = dm.getAllListNames();
    EXPECT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "Deck1");

    EXPECT_FALSE(dm.createList("Deck1"));
    EXPECT_EQ(dm.getAllListNames().size(), 1u);

    EXPECT_TRUE(dm.createList("Deck2"));
    EXPECT_NE(dm.getList("Deck2"), nullptr);
    names = dm.getAllListNames();
    EXPECT_EQ(names.size(), 2u);
    EXPECT_TRUE(std::find(names.begin(), names.end(), "Deck1") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "Deck2") != names.end());

    EXPECT_TRUE(dm.deleteList("Deck1"));
    EXPECT_EQ(dm.getList("Deck1"), nullptr);
    names = dm.getAllListNames();
    EXPECT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "Deck2");

    EXPECT_FALSE(dm.deleteList("Deck1"));
    EXPECT_FALSE(dm.deleteList("NonExistentDeck"));
    EXPECT_EQ(dm.getAllListNames().size(), 1u);
}

TEST(DeckManagerTest, AddRemoveUpdateCardInListFacade) {
    DeckManager dm(nullptr);
    dm.createList("TestDeck");

    auto card1 = test_utils::createTestFlashcard("c1", "Front1", "Back1");
    auto card2 = test_utils::createTestFlashcard("c2", "Front2", "Back2");

    EXPECT_TRUE(dm.addCardToList("TestDeck", card1));
    EXPECT_EQ(dm.getList("TestDeck")->size(), 1u);
    EXPECT_TRUE(dm.addCardToList("TestDeck", card2));
    EXPECT_EQ(dm.getList("TestDeck")->size(), 2u);
    EXPECT_FALSE(dm.addCardToList("NonExistent", test_utils::createTestFlashcard("fail", "", "")));

    EXPECT_TRUE(dm.updateCardInList("TestDeck", "c1", "Updated Front1", "Updated Back1"));
    EXPECT_EQ(dm.getList("TestDeck")->getCard("c1")->text_front, "Updated Front1");
    EXPECT_FALSE(dm.updateCardInList("NonExistent", "c1", "", ""));
    EXPECT_FALSE(dm.updateCardInList("TestDeck", "c99", "", ""));

    EXPECT_TRUE(dm.removeCardFromList("TestDeck", "c1"));
    EXPECT_EQ(dm.getList("TestDeck")->size(), 1u);
    EXPECT_EQ(dm.getList("TestDeck")->getCard("c1"), nullptr);
    EXPECT_FALSE(dm.removeCardFromList("NonExistent", "c2"));
    EXPECT_FALSE(dm.removeCardFromList("TestDeck", "c99"));
}


TEST(DeckManagerTest, MoveCardBetweenLists) {
    DeckManager dm(nullptr);
    dm.createList("SourceDeck");
    dm.createList("TargetDeck");
    dm.createList("OtherDeck");

    auto card1 = test_utils::createTestFlashcard("move1", "Card1", "Data1");
    auto card2 = test_utils::createTestFlashcard("move2", "Card2", "Data2");
    dm.addCardToList("SourceDeck", card1);
    dm.addCardToList("SourceDeck", card2);

    ASSERT_EQ(dm.getList("SourceDeck")->size(), 2u);
    ASSERT_EQ(dm.getList("TargetDeck")->size(), 0u);

    EXPECT_TRUE(dm.moveCard("move1", "SourceDeck", "TargetDeck"));

    EXPECT_EQ(dm.getList("SourceDeck")->size(), 1u);
    EXPECT_EQ(dm.getList("SourceDeck")->getCard("move1"), nullptr);
    EXPECT_EQ(dm.getList("TargetDeck")->size(), 1u);
    EXPECT_NE(dm.getList("TargetDeck")->getCard("move1"), nullptr);
    EXPECT_EQ(dm.getList("TargetDeck")->getCard("move1"), card1);

    EXPECT_FALSE(dm.moveCard("nonexistent", "SourceDeck", "TargetDeck"));
    EXPECT_EQ(dm.getList("SourceDeck")->size(), 1u);
    EXPECT_EQ(dm.getList("TargetDeck")->size(), 1u);

    EXPECT_FALSE(dm.moveCard("move2", "SourceDeck", "NonExistentTarget"));

    EXPECT_FALSE(dm.moveCard("move2", "NonExistentSource", "TargetDeck"));

    EXPECT_FALSE(dm.moveCard("move2", "SourceDeck", "SourceDeck"));

    dm.addCardToList("TargetDeck", card2);
    EXPECT_FALSE(dm.moveCard("move2", "SourceDeck", "TargetDeck"));
    EXPECT_EQ(dm.getList("SourceDeck")->size(), 1u);
    EXPECT_EQ(dm.getList("TargetDeck")->size(), 2u);
    EXPECT_NE(dm.getList("SourceDeck")->getCard("move2"), nullptr);
    EXPECT_NE(dm.getList("TargetDeck")->getCard("move2"), nullptr);

    dm.getList("TargetDeck")->clear();
    dm.getList("TargetDeck")->addCard(card1);
    EXPECT_FALSE(dm.moveCard("move1", "SourceDeck", "TargetDeck"));
    dm.getList("SourceDeck")->addCard(card1);
    dm.getList("TargetDeck")->clear();
    dm.getList("TargetDeck")->addCard(test_utils::createTestFlashcard("move1", "Dummy", "Dummy"));
    EXPECT_EQ(dm.getList("SourceDeck")->size(), 2u);
    EXPECT_EQ(dm.getList("TargetDeck")->size(), 1u);
    EXPECT_FALSE(dm.moveCard("move1", "SourceDeck", "TargetDeck"));
    EXPECT_EQ(dm.getList("SourceDeck")->size(), 2u);
    EXPECT_EQ(dm.getList("TargetDeck")->size(), 1u);

}

TEST(DeckManagerTest, BulkMoveCards) {
    DeckManager dm(nullptr);
    dm.createList("Source");
    dm.createList("Target");

    auto c1 = test_utils::createTestFlashcard("c1", "1", "one");
    auto c2 = test_utils::createTestFlashcard("c2", "2", "two");
    auto c3 = test_utils::createTestFlashcard("c3", "3", "three");
    auto c4 = test_utils::createTestFlashcard("c4", "4", "four");

    dm.addCardToList("Source", c1);
    dm.addCardToList("Source", c2);
    dm.addCardToList("Source", c3);
    dm.addCardToList("Source", c4);

    ASSERT_EQ(dm.getList("Source")->size(), 4u);
    ASSERT_EQ(dm.getList("Target")->size(), 0u);

    std::vector<std::string> cardsToMove = {"c1", "c3", "nonexistent_card"};
    size_t movedCount = dm.moveCards(cardsToMove, "Source", "Target");
    EXPECT_EQ(movedCount, 2u);

    EXPECT_EQ(dm.getList("Source")->size(), 2u);
    EXPECT_EQ(dm.getList("Target")->size(), 2u);

    EXPECT_EQ(dm.getList("Source")->getCard("c1"), nullptr);
    EXPECT_NE(dm.getList("Source")->getCard("c2"), nullptr);
    EXPECT_EQ(dm.getList("Source")->getCard("c3"), nullptr);
    EXPECT_NE(dm.getList("Source")->getCard("c4"), nullptr);

    EXPECT_NE(dm.getList("Target")->getCard("c1"), nullptr);
    EXPECT_EQ(dm.getList("Target")->getCard("c2"), nullptr);
    EXPECT_NE(dm.getList("Target")->getCard("c3"), nullptr);
    EXPECT_EQ(dm.getList("Target")->getCard("c4"), nullptr);

    EXPECT_EQ(dm.getList("Target")->getCard("c1"), c1);
    EXPECT_EQ(dm.getList("Target")->getCard("c3"), c3);

    EXPECT_EQ(dm.moveCards({"c2"}, "Source", "NonExistent"), 0u);
    EXPECT_EQ(dm.moveCards({"c2"}, "NonExistent", "Target"), 0u);
    EXPECT_EQ(dm.moveCards({"c2"}, "Source", "Source"), 0u);

    dm.addCardToList("Source", c1);
    dm.addCardToList("Source", c2);
    dm.addCardToList("Source", c4);
    EXPECT_EQ(dm.getList("Source")->size(), 3u);
    EXPECT_EQ(dm.getList("Target")->size(), 2u);

    std::vector<std::string> cardsToMoveAgain = {"c1", "c2", "c4"};
    size_t movedCount2 = dm.moveCards(cardsToMoveAgain, "Source", "Target");
    EXPECT_EQ(movedCount2, 2u);

    EXPECT_EQ(dm.getList("Source")->size(), 1u);
    EXPECT_NE(dm.getList("Source")->getCard("c1"), nullptr);

    EXPECT_EQ(dm.getList("Target")->size(), 4u);
    EXPECT_NE(dm.getList("Target")->getCard("c1"), nullptr);
    EXPECT_NE(dm.getList("Target")->getCard("c2"), nullptr);
    EXPECT_NE(dm.getList("Target")->getCard("c3"), nullptr);
    EXPECT_NE(dm.getList("Target")->getCard("c4"), nullptr);

}

TEST(DeckManagerTest, ImportCardsFromFile) {
    DeckManager dm(nullptr);
    dm.createList("ImportTest");

    std::filesystem::path testFilePath = std::filesystem::temp_directory_path() / "test_import.csv";
    {
        std::ofstream testFile(testFilePath, std::ios::binary);
        ASSERT_TRUE(testFile.is_open());
        
        testFile << '\xEF' << '\xBB' << '\xBF';
        
        testFile << "Word;Translation\n";
        testFile << "Apple;Jabłko\n";
        testFile << "Dog;Pies\n";
        testFile << "Cat;Kot\n";
        
        testFile.close();
    }

    size_t importedCount = dm.importCardsFromFile("ImportTest", testFilePath, ';', true);
    
    EXPECT_EQ(importedCount, 3u);
    EXPECT_EQ(dm.getList("ImportTest")->size(), 3u);

    auto cards = dm.getList("ImportTest")->getAllCards();
    bool foundApple = false;
    bool foundDog = false;
    bool foundCat = false;

    for (const auto& card : cards) {
        if (card->text_front == "Apple" && card->text_back == "Jabłko") {
            foundApple = true;
        }
        if (card->text_front == "Dog" && card->text_back == "Pies") {
            foundDog = true;
        }
        if (card->text_front == "Cat" && card->text_back == "Kot") {
            foundCat = true;
        }
    }

    EXPECT_TRUE(foundApple);
    EXPECT_TRUE(foundDog);
    EXPECT_TRUE(foundCat);

    std::filesystem::remove(testFilePath);
}

TEST(DeckManagerTest, ImportCardsFromFile_ReimportSameContentAddsAgain) {
    DeckManager dm(nullptr);
    dm.createList("ImportRepeat");

    std::filesystem::path testFilePath = std::filesystem::temp_directory_path() / "test_import_repeat.csv";
    {
        std::ofstream testFile(testFilePath, std::ios::binary);
        ASSERT_TRUE(testFile.is_open());
        testFile << "Word;Translation\n";
        testFile << "Apple;Jablko\n";
        testFile.close();
    }

    const size_t firstImport = dm.importCardsFromFile("ImportRepeat", testFilePath, ';', true);
    const size_t secondImport = dm.importCardsFromFile("ImportRepeat", testFilePath, ';', true);

    EXPECT_EQ(firstImport, 1u);
    EXPECT_EQ(secondImport, 1u);
    EXPECT_EQ(dm.getList("ImportRepeat")->size(), 2u);

    auto cards = dm.getList("ImportRepeat")->getAllCards();
    ASSERT_EQ(cards.size(), 2u);
    EXPECT_EQ(cards[0]->text_front, "Apple");
    EXPECT_EQ(cards[1]->text_front, "Apple");
    EXPECT_NE(cards[0]->card_id, cards[1]->card_id);

    std::filesystem::remove(testFilePath);
}

TEST(DeckManagerTest, ImportCardsFromFileSkipsMalformedLines) {
    DeckManager dm(nullptr);
    dm.createList("MalformedTest");

    std::filesystem::path testFilePath = std::filesystem::temp_directory_path() / "test_malformed.csv";
    {
        std::ofstream testFile(testFilePath);
        ASSERT_TRUE(testFile.is_open());
        
        testFile << "Front;Back\n";
        testFile << "ValidFront;ValidBack\n";
        testFile << "OnlyOneLine\n";
        testFile << "Three;Fields;Here\n";
        testFile << "  ;  \n";
        testFile << "Good;Data\n";
        testFile << "\n";
        testFile << "Another;Valid;Extra\n";
        testFile << "Last;One\n";
        
        testFile.close();
    }

    size_t importedCount = dm.importCardsFromFile("MalformedTest", testFilePath, ';', true);
    
    EXPECT_EQ(importedCount, 6u);
    EXPECT_EQ(dm.getList("MalformedTest")->size(), 6u);

    auto cards = dm.getList("MalformedTest")->getAllCards();
    bool foundValid1 = false;
    bool foundValid2 = false;
    bool foundValid3 = false;
    bool foundOnlyOne = false;
    bool foundThreeFields = false;
    bool foundAnotherExtra = false;

    for (const auto& card : cards) {
        if (card->text_front == "ValidFront" && card->text_back == "ValidBack") {
            foundValid1 = true;
        }
        if (card->text_front == "Good" && card->text_back == "Data") {
            foundValid2 = true;
        }
        if (card->text_front == "Last" && card->text_back == "One") {
            foundValid3 = true;
        }
        if (card->text_front == "OnlyOneLine" && card->text_back.empty()) {
            foundOnlyOne = true;
        }
        if (card->text_front == "Three" && card->text_back == "Fields;Here") {
            foundThreeFields = true;
        }
        if (card->text_front == "Another" && card->text_back == "Valid;Extra") {
            foundAnotherExtra = true;
        }
    }

    EXPECT_TRUE(foundValid1);
    EXPECT_TRUE(foundValid2);
    EXPECT_TRUE(foundValid3);
    EXPECT_TRUE(foundOnlyOne);
    EXPECT_TRUE(foundThreeFields);
    EXPECT_TRUE(foundAnotherExtra);

    std::filesystem::remove(testFilePath);
}

TEST(DeckManagerTest, ImportCardsFromFileThrowsOnInvalidFile) {
    DeckManager dm(nullptr);
    dm.createList("ErrorTest");

    std::filesystem::path nonExistentFile = "/nonexistent/path/to/file.csv";
    
    EXPECT_THROW({
        dm.importCardsFromFile("ErrorTest", nonExistentFile, ',', false);
    }, std::runtime_error);
}

TEST(DeckManagerTest, ImportCardsFromFileReturnsZeroForNonExistentList) {
    DeckManager dm(nullptr);

    std::filesystem::path testFilePath = std::filesystem::temp_directory_path() / "test_no_list.csv";
    {
        std::ofstream testFile(testFilePath);
        testFile << "Front,Back\n";
        testFile << "Test,Data\n";
        testFile.close();
    }

    size_t importedCount = dm.importCardsFromFile("NonExistentList", testFilePath, ',', true);
    
    EXPECT_EQ(importedCount, 0u);

    std::filesystem::remove(testFilePath);
}
