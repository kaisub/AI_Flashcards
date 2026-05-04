#include "gtest/gtest.h"
#include "app/presenters/DeckEditorPresenter.hpp"
#include "app/views/IDeckEditorView.hpp"
#include "core/IDeckManager.hpp"
#include "core/Flashcard.hpp"
#include <unordered_map>

namespace app {

namespace {

class SpyDeckEditorView : public IDeckEditorView {
public:
    void run() override {
        runCount++;
    }

    void setDeck(std::shared_ptr<core::FlashcardList> deck) override {
        lastDeck = std::move(deck);
        setDeckCount++;
    }

    void setAvailableLists(const std::vector<std::string>& listNames) override {
        availableLists = listNames;
        setAvailableListsCount++;
    }

    int runCount = 0;
    int setDeckCount = 0;
    int setAvailableListsCount = 0;
    std::shared_ptr<core::FlashcardList> lastDeck;
    std::vector<std::string> availableLists;
};

class FakeDeckManager : public core::IDeckManager {
public:
    FakeDeckManager() = default;

    void addList(const std::string& name, const std::filesystem::path& path,
                 const std::vector<std::shared_ptr<core::Flashcard>>& cards = {}) {
        auto list = std::make_shared<core::FlashcardList>(name);
        for (const auto& card : cards) {
            list->addCard(card);
        }
        lists[name] = list;
        pathToName[path] = name;
        allPaths.push_back(path);
    }

    void addAvailablePath(const std::filesystem::path& path) {
        allPaths.push_back(path);
    }

    std::shared_ptr<core::FlashcardList> getList(const std::string& listName) const override {
        auto it = lists.find(listName);
        if (it == lists.end()) {
            return nullptr;
        }
        return it->second;
    }

    std::vector<std::filesystem::path> getAllAvailableLists() const override {
        return allPaths;
    }

    std::shared_ptr<core::FlashcardList> loadList(const std::filesystem::path& path) override {
        auto nameIt = pathToName.find(path);
        if (nameIt == pathToName.end()) {
            return nullptr;
        }
        auto listIt = lists.find(nameIt->second);
        if (listIt == lists.end()) {
            return nullptr;
        }
        return listIt->second;
    }

    bool addCardToList(const std::string& listName, std::shared_ptr<core::Flashcard> card) override {
        auto list = getList(listName);
        return list ? list->addCard(std::move(card)) : false;
    }

    bool removeCardFromList(const std::string& listName, const std::string& cardId) override {
        auto list = getList(listName);
        return list ? list->removeCard(cardId) : false;
    }

    bool updateCardInList(const std::string& listName, const std::string& cardId,
                          const std::string& newFront, const std::string& newBack) override {
        auto list = getList(listName);
        return list ? list->updateCard(cardId, newFront, newBack) : false;
    }

    size_t removeCardsFromList(const std::string& listName, const std::vector<std::string>& cardIds) override {
        auto list = getList(listName);
        return list ? list->removeCards(cardIds) : 0;
    }

    size_t moveCards(const std::vector<std::string>& cardIds, const std::string& sourceListName,
                     const std::string& targetListName) override {
        auto source = getList(sourceListName);
        auto target = getList(targetListName);
        if (!source || !target || sourceListName == targetListName) {
            return 0;
        }

        size_t moved = 0;
        for (const auto& cardId : cardIds) {
            auto card = source->getCard(cardId);
            if (!card) {
                continue;
            }
            if (target->addCard(card)) {
                source->removeCard(cardId);
                moved++;
            }
        }
        return moved;
    }

    size_t addCardsToList(const std::string& listName,
                          const std::vector<std::shared_ptr<core::Flashcard>>& cards) override {
        auto list = getList(listName);
        return list ? list->importCardsFrom(cards) : 0;
    }

    bool saveList(const std::string& listName) override {
        savedLists.push_back(listName);
        return lists.find(listName) != lists.end();
    }

    std::vector<std::string> savedLists;

    // Dummy implementations for unused interface methods
    bool createFolder(const std::filesystem::path&) override { return false; }
    bool deleteFolder(const std::filesystem::path&) override { return false; }
    bool renameFolder(const std::filesystem::path&, const std::filesystem::path&) override { return false; }
    bool moveListFile(const std::filesystem::path&, const std::filesystem::path&) override { return false; }
    bool deleteListFile(const std::filesystem::path&) override { return false; }
    bool createList(const std::string&, const std::filesystem::path&) override { return false; }
    bool createList(const std::string&) override { return false; }
    bool deleteList(const std::string&) override { return false; }
    std::vector<std::string> getAllListNames() const override { return {}; }
    bool moveCard(const std::string&, const std::string&, const std::string&) override { return false; }
    size_t importCardsFromFile(const std::string&, const std::filesystem::path&, char, bool) override { return 0; }

private:
    std::unordered_map<std::string, std::shared_ptr<core::FlashcardList>> lists;
    std::unordered_map<std::filesystem::path, std::string> pathToName;
    std::vector<std::filesystem::path> allPaths;
};

} // namespace

class DeckEditorPresenterTest : public ::testing::Test {
protected:
    std::shared_ptr<SpyDeckEditorView> view;
    std::unique_ptr<FakeDeckManager> deckManager;
    std::unique_ptr<presenters::DeckEditorPresenter> presenter;

    void SetUp() override {
        view = std::make_shared<SpyDeckEditorView>();
        deckManager = std::make_unique<FakeDeckManager>();
        presenter = std::make_unique<presenters::DeckEditorPresenter>(view, deckManager.get());
    }
};

TEST_F(DeckEditorPresenterTest, StartPopulatesView) {
    const std::filesystem::path currentPath = "path/to/deck.json";
    const std::filesystem::path otherPath = "path/to/other.json";
    deckManager->addList("MyDeck", currentPath);
    deckManager->addList("OtherDeck", otherPath);

    presenter->start("MyDeck", currentPath);

    EXPECT_EQ(view->setDeckCount, 1);
    EXPECT_EQ(view->setAvailableListsCount, 1);
    EXPECT_EQ(view->runCount, 1);
    ASSERT_EQ(view->lastDeck, deckManager->getList("MyDeck"));
    ASSERT_EQ(view->availableLists.size(), 1U);
    EXPECT_EQ(view->availableLists[0], otherPath.string());
}

TEST_F(DeckEditorPresenterTest, OnStartStudyWithEmptyDeckDoesNothing) {
    deckManager->addList("EmptyDeck", "path/to/deck.json");
    presenter->start("EmptyDeck", "path/to/deck.json");

    bool callbackCalled = false;
    presenter->onStartStudy = [&]() { callbackCalled = true; };

    ASSERT_TRUE(view->triggerStartStudy());

    EXPECT_FALSE(callbackCalled);
}

TEST_F(DeckEditorPresenterTest, OnAddCardRefreshesView) {
    deckManager->addList("MyDeck", "path/to/deck.json");
    presenter->start("MyDeck", "path/to/deck.json");

    auto list = deckManager->getList("MyDeck");
    ASSERT_NE(list, nullptr);
    const auto before = list->size();
    ASSERT_TRUE(view->triggerAddCard("front", "back"));

    EXPECT_EQ(list->size(), before + 1);
    EXPECT_EQ(view->setDeckCount, 2); // once in start(), once after add
    EXPECT_EQ(deckManager->savedLists.size(), 1U);
    EXPECT_EQ(deckManager->savedLists.back(), "MyDeck");
}

TEST_F(DeckEditorPresenterTest, OnUpdateCardRefreshesView) {
    auto card = std::make_shared<core::Flashcard>();
    card->id = "card-1";
    card->text_front = "front";
    card->text_back = "back";
    deckManager->addList("MyDeck", "path/to/deck.json", {card});
    presenter->start("MyDeck", "path/to/deck.json");

    ASSERT_TRUE(view->triggerUpdateCard("card-1", "new_front", "new_back"));

    auto list = deckManager->getList("MyDeck");
    ASSERT_NE(list, nullptr);
    auto updated = list->getCard("card-1");
    ASSERT_NE(updated, nullptr);
    EXPECT_EQ(updated->text_front, "new_front");
    EXPECT_EQ(updated->text_back, "new_back");
    EXPECT_EQ(view->setDeckCount, 2); // once in start(), once after update
    EXPECT_EQ(deckManager->savedLists.size(), 1U);
    EXPECT_EQ(deckManager->savedLists.back(), "MyDeck");
}

TEST_F(DeckEditorPresenterTest, OnDeleteSelectedRefreshesView) {
    auto card1 = std::make_shared<core::Flashcard>();
    card1->id = "card-1";
    auto card2 = std::make_shared<core::Flashcard>();
    card2->id = "card-2";
    deckManager->addList("MyDeck", "path/to/deck.json", {card1, card2});
    presenter->start("MyDeck", "path/to/deck.json");

    std::vector<std::string> ids_to_delete = {"card-1", "card-2"};
    ASSERT_TRUE(view->triggerDeleteSelected(ids_to_delete));

    auto list = deckManager->getList("MyDeck");
    ASSERT_NE(list, nullptr);
    EXPECT_EQ(list->size(), 0U);
    EXPECT_EQ(view->setDeckCount, 2); // once in start(), once after delete
    EXPECT_EQ(deckManager->savedLists.size(), 1U);
    EXPECT_EQ(deckManager->savedLists.back(), "MyDeck");
}

TEST_F(DeckEditorPresenterTest, OnCopySelectedCallsDeckManager) {
    auto sourceList = std::make_shared<core::FlashcardList>("SourceDeck");
    auto card = std::make_shared<core::Flashcard>();
    card->id = "card-1";
    card->text_front = "front";
    card->text_back = "back";
    card->state_Front_to_Back = core::CardState::Known;
    card->state_Back_to_Front = core::CardState::Known;
    sourceList->addCard(card);

    deckManager->addList("SourceDeck", "path/to/source.json", {card});
    deckManager->addList("TargetDeck", "path/to/target.json");
    presenter->start("SourceDeck", "path/to/source.json");

    ASSERT_TRUE(view->triggerCopySelected({"card-1"}, 0));

    auto targetList = deckManager->getList("TargetDeck");
    ASSERT_NE(targetList, nullptr);
    ASSERT_EQ(targetList->size(), 1U);
    auto copied = targetList->getAllCards().front();
    EXPECT_EQ(copied->text_front, "front");
    EXPECT_EQ(copied->text_back, "back");
    EXPECT_EQ(copied->state_Front_to_Back, core::CardState::New);
    EXPECT_EQ(copied->state_Back_to_Front, core::CardState::New);
    EXPECT_EQ(deckManager->savedLists.size(), 1U);
    EXPECT_EQ(deckManager->savedLists.back(), "TargetDeck");
}

TEST_F(DeckEditorPresenterTest, StartWithMissingListDoesNotTouchView) {
    presenter->start("MissingDeck", "path/to/missing.json");

    EXPECT_EQ(view->setDeckCount, 0);
    EXPECT_EQ(view->setAvailableListsCount, 0);
    EXPECT_EQ(view->runCount, 0);
}

TEST_F(DeckEditorPresenterTest, OnCopySelectedWithInvalidIndexDoesNothing) {
    auto card = std::make_shared<core::Flashcard>();
    card->id = "card-1";
    deckManager->addList("SourceDeck", "path/to/source.json", {card});
    deckManager->addList("TargetDeck", "path/to/target.json");
    presenter->start("SourceDeck", "path/to/source.json");

    ASSERT_TRUE(view->triggerCopySelected({"card-1"}, -1));
    ASSERT_TRUE(view->triggerCopySelected({"card-1"}, 99));

    auto targetList = deckManager->getList("TargetDeck");
    ASSERT_NE(targetList, nullptr);
    EXPECT_EQ(targetList->size(), 0U);
}

TEST_F(DeckEditorPresenterTest, OnCopySelectedWithMissingTargetListDoesNothing) {
    auto card = std::make_shared<core::Flashcard>();
    card->id = "card-1";
    deckManager->addList("SourceDeck", "path/to/source.json", {card});
    deckManager->addAvailablePath("path/to/missing_target.json");
    presenter->start("SourceDeck", "path/to/source.json");

    ASSERT_TRUE(view->triggerCopySelected({"card-1"}, 0));

    auto sourceList = deckManager->getList("SourceDeck");
    ASSERT_NE(sourceList, nullptr);
    EXPECT_EQ(sourceList->size(), 1U);
}

TEST_F(DeckEditorPresenterTest, OnMoveSelectedWithNoMovableCardsDoesNotRefresh) {
    auto sourceCard = std::make_shared<core::Flashcard>();
    sourceCard->id = "source-card";
    deckManager->addList("SourceDeck", "path/to/source.json", {sourceCard});
    deckManager->addList("TargetDeck", "path/to/target.json");
    presenter->start("SourceDeck", "path/to/source.json");

    const int baselineRefreshes = view->setDeckCount;
    ASSERT_TRUE(view->triggerMoveSelected({"missing-card"}, 0));

    EXPECT_EQ(view->setDeckCount, baselineRefreshes);
    auto sourceList = deckManager->getList("SourceDeck");
    auto targetList = deckManager->getList("TargetDeck");
    ASSERT_NE(sourceList, nullptr);
    ASSERT_NE(targetList, nullptr);
    EXPECT_EQ(sourceList->size(), 1U);
    EXPECT_EQ(targetList->size(), 0U);
}

TEST_F(DeckEditorPresenterTest, OnMoveSelectedMovesCardsAndSavesBothLists) {
    auto sourceCard = std::make_shared<core::Flashcard>();
    sourceCard->id = "card-1";
    deckManager->addList("SourceDeck", "path/to/source.json", {sourceCard});
    deckManager->addList("TargetDeck", "path/to/target.json");
    presenter->start("SourceDeck", "path/to/source.json");

    ASSERT_TRUE(view->triggerMoveSelected({"card-1"}, 0));

    auto targetList = deckManager->getList("TargetDeck");
    ASSERT_EQ(targetList->size(), 1U);
    EXPECT_EQ(deckManager->savedLists.size(), 2U); // Should save both Source and Target
}

TEST_F(DeckEditorPresenterTest, OnExitToBrowserFiresCallbackAndSaves) {
    deckManager->addList("MyDeck", "path/to/deck.json");
    presenter->start("MyDeck", "path/to/deck.json");

    bool callbackCalled = false;
    presenter->onExitToBrowser = [&]() { callbackCalled = true; };
    ASSERT_TRUE(view->triggerExitToBrowser());
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(deckManager->savedLists.size(), 1U);
    EXPECT_EQ(deckManager->savedLists.back(), "MyDeck");
}

TEST_F(DeckEditorPresenterTest, OnImportRequestedImportsAndSaves) {
    deckManager->addList("MyDeck", "path/to/deck.json");
    presenter->start("MyDeck", "path/to/deck.json");

    const int baselineRefreshes = view->setDeckCount;
    ASSERT_TRUE(view->triggerImportRequested("test.csv", ';', true));

    EXPECT_EQ(view->setDeckCount, baselineRefreshes + 1);
    EXPECT_EQ(deckManager->savedLists.size(), 1U);
    EXPECT_EQ(deckManager->savedLists.back(), "MyDeck");
}

} // namespace app