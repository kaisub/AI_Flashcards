#include "gtest/gtest.h"
#include "app/presenters/StudySessionPresenter.hpp"
#include "app/views/IStudySessionView.hpp"
#include "core/IDeckManager.hpp"
#include "core/Flashcard.hpp"
#include <unordered_map>

namespace app {

namespace {

class SpyStudySessionView : public IStudySessionView {
public:
    void run() override {
        runCount++;
    }

    void showCard(const core::Flashcard& card, bool flipped) override {
        showCardCount++;
        lastShownCardId = card.card_id;
        lastShownFlipped = flipped;
    }

    void showSessionComplete() override {
        showSessionCompleteCount++;
    }

    void setAvailableLists(const std::vector<std::string>& listNames) override {
        availableLists = listNames;
        setAvailableListsCount++;
    }

    int runCount = 0;
    int showCardCount = 0;
    int showSessionCompleteCount = 0;
    int setAvailableListsCount = 0;
    bool lastShownFlipped = false;
    std::string lastShownCardId;
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

    bool saveList(const std::string& listName) override {
        savedLists.push_back(listName);
        return lists.find(listName) != lists.end();
    }

    bool updateCardInList(const std::string& listName, const std::string& cardId,
                          const std::string& newFront, const std::string& newBack) override {
        if (forceUpdateFailure) {
            return false;
        }
        auto list = getList(listName);
        return list ? list->updateCard(cardId, newFront, newBack) : false;
    }

    bool removeCardFromList(const std::string& listName, const std::string& cardId) override {
        if (forceRemoveFailure) {
            return false;
        }
        auto list = getList(listName);
        return list ? list->removeCard(cardId) : false;
    }

    std::shared_ptr<core::FlashcardList> loadList(const std::filesystem::path& path) override {
        auto it = pathToName.find(path);
        if (it == pathToName.end()) {
            return nullptr;
        }
        return getList(it->second);
    }

    bool addCardToList(const std::string& listName, std::shared_ptr<core::Flashcard> card) override {
        auto list = getList(listName);
        return list ? list->addCard(std::move(card)) : false;
    }

    std::vector<std::string> savedLists;
    bool forceUpdateFailure = false;
    bool forceRemoveFailure = false;

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
    size_t addCardsToList(const std::string&, const std::vector<std::shared_ptr<core::Flashcard>>&) override { return 0; }
    size_t removeCardsFromList(const std::string&, const std::vector<std::string>&) override { return 0; }
    size_t moveCards(const std::vector<std::string>&, const std::string&, const std::string&) override { return 0; }
    size_t importCardsFromFile(const std::string&, const std::filesystem::path&, char, bool) override { return 0; }

private:
    std::unordered_map<std::string, std::shared_ptr<core::FlashcardList>> lists;
    std::unordered_map<std::filesystem::path, std::string> pathToName;
    std::vector<std::filesystem::path> allPaths;
};

} // namespace

class StudySessionPresenterTest : public ::testing::Test {
protected:
    std::shared_ptr<SpyStudySessionView> view;
    std::unique_ptr<FakeDeckManager> deckManager;
    std::unique_ptr<presenters::StudySessionPresenter> presenter;

    void SetUp() override {
        view = std::make_shared<SpyStudySessionView>();
        deckManager = std::make_unique<FakeDeckManager>();
        presenter = std::make_unique<presenters::StudySessionPresenter>(view, deckManager.get());
    }
};

TEST_F(StudySessionPresenterTest, StartWithEmptyDeckExits) {
    deckManager->addList("Empty", "path/to/empty.json");

    bool exitCalled = false;
    presenter->onExitRequested = [&]() { exitCalled = true; };

    core::SessionConfig config;
    presenter->start("Empty", "path/to/empty.json", config);

    EXPECT_TRUE(exitCalled);
    EXPECT_EQ(view->setAvailableListsCount, 0);
}

TEST_F(StudySessionPresenterTest, HandleStepWithoutStartRequestsExit) {
    bool exitCalled = false;
    presenter->onExitRequested = [&]() { exitCalled = true; };

    presenter->handleStep();

    EXPECT_TRUE(exitCalled);
    EXPECT_EQ(view->showCardCount, 0);
    EXPECT_EQ(view->showSessionCompleteCount, 0);
    EXPECT_EQ(view->runCount, 0);
}

TEST_F(StudySessionPresenterTest, HandleStepShowsCardWhenAvailable) {
    auto card = std::make_shared<core::Flashcard>();
    card->card_id = "card-1";
    deckManager->addList("MyDeck", "path/to/deck.json", {card});

    core::SessionConfig config;
    presenter->start("MyDeck", "path/to/deck.json", config);

    presenter->handleStep();

    EXPECT_EQ(view->showCardCount, 1);
    EXPECT_EQ(view->lastShownCardId, "card-1");
    EXPECT_EQ(view->lastShownFlipped, false);
    EXPECT_EQ(view->runCount, 1);
}

TEST_F(StudySessionPresenterTest, OnExitRequestedSavesList) {
    auto card = std::make_shared<core::Flashcard>();
    card->card_id = "card-1";
    deckManager->addList("MyDeck", "path/to/deck.json", {card});

    core::SessionConfig config;
    presenter->start("MyDeck", "path/to/deck.json", config);

    bool exitCallbackCalled = false;
    presenter->onExitRequested = [&]() { exitCallbackCalled = true; };

    ASSERT_TRUE(view->triggerExitRequested());

    ASSERT_FALSE(deckManager->savedLists.empty());
    EXPECT_EQ(deckManager->savedLists.back(), "MyDeck");
    EXPECT_TRUE(exitCallbackCalled);
}

TEST_F(StudySessionPresenterTest, OnEditRequestedUpdatesCard) {
    auto card = std::make_shared<core::Flashcard>();
    card->card_id = "card-to-edit";
    card->text_front = "old_front";
    card->text_back = "old_back";
    deckManager->addList("MyDeck", "path/to/deck.json", {card});

    core::SessionConfig config;
    presenter->start("MyDeck", "path/to/deck.json", config);
    presenter->handleStep(); // Load the first card

    ASSERT_TRUE(view->triggerEditRequested("new_front", "new_back"));

    auto list = deckManager->getList("MyDeck");
    ASSERT_NE(list, nullptr);
    auto edited = list->getCard("card-to-edit");
    ASSERT_NE(edited, nullptr);
    EXPECT_EQ(edited->text_front, "new_front");
    EXPECT_EQ(edited->text_back, "new_back");
    EXPECT_EQ(view->showCardCount, 2); // one in handleStep, one after edit
    EXPECT_EQ(deckManager->savedLists.size(), 1U);
    EXPECT_EQ(deckManager->savedLists.back(), "MyDeck");
}

TEST_F(StudySessionPresenterTest, OnDeleteRequestedRemovesCard) {
    auto card = std::make_shared<core::Flashcard>();
    card->card_id = "card-to-delete";
    deckManager->addList("MyDeck", "path/to/deck.json", {card});

    core::SessionConfig config;
    presenter->start("MyDeck", "path/to/deck.json", config);
    presenter->handleStep(); // Load the first card

    ASSERT_TRUE(view->triggerDeleteRequested());

    auto list = deckManager->getList("MyDeck");
    ASSERT_NE(list, nullptr);
    EXPECT_EQ(list->getCard("card-to-delete"), nullptr);
    EXPECT_EQ(deckManager->savedLists.size(), 1U);
    EXPECT_EQ(deckManager->savedLists.back(), "MyDeck");
}

TEST_F(StudySessionPresenterTest, OnCopyRequestedWithInvalidIndexDoesNothing) {
    auto card = std::make_shared<core::Flashcard>();
    card->card_id = "card-to-copy";
    card->text_front = "front";
    card->text_back = "back";
    deckManager->addList("Source", "path/to/source.json", {card});
    deckManager->addList("Target", "path/to/target.json");

    core::SessionConfig config;
    presenter->start("Source", "path/to/source.json", config);
    presenter->handleStep();

    ASSERT_TRUE(view->triggerCopyRequested(-1));
    ASSERT_TRUE(view->triggerCopyRequested(99));

    auto target = deckManager->getList("Target");
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->size(), 0U);
}

TEST_F(StudySessionPresenterTest, OnCopyRequestedWithMissingTargetListDoesNothing) {
    auto card = std::make_shared<core::Flashcard>();
    card->card_id = "card-to-copy";
    deckManager->addList("Source", "path/to/source.json", {card});
    deckManager->addAvailablePath("path/to/missing_target.json");

    core::SessionConfig config;
    presenter->start("Source", "path/to/source.json", config);
    presenter->handleStep();

    ASSERT_TRUE(view->triggerCopyRequested(0));

    auto source = deckManager->getList("Source");
    ASSERT_NE(source, nullptr);
    EXPECT_EQ(source->size(), 1U);
}

TEST_F(StudySessionPresenterTest, OnCopyRequestedCopiesCardAndSavesTargetList) {
    auto card = std::make_shared<core::Flashcard>();
    card->card_id = "card-to-copy";
    deckManager->addList("Source", "path/to/source.json", {card});
    deckManager->addList("Target", "path/to/target.json");

    core::SessionConfig config;
    presenter->start("Source", "path/to/source.json", config);
    presenter->handleStep();

    ASSERT_TRUE(view->triggerCopyRequested(0));

    auto target = deckManager->getList("Target");
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->size(), 1U);
    EXPECT_EQ(deckManager->savedLists.size(), 1U);
    EXPECT_EQ(deckManager->savedLists.back(), "Target");
}

TEST_F(StudySessionPresenterTest, OnEditRequestedWhenUpdateFailsDoesNotRefreshCard) {
    auto card = std::make_shared<core::Flashcard>();
    card->card_id = "card-to-edit";
    card->text_front = "old_front";
    card->text_back = "old_back";
    deckManager->addList("MyDeck", "path/to/deck.json", {card});

    core::SessionConfig config;
    presenter->start("MyDeck", "path/to/deck.json", config);
    presenter->handleStep();

    deckManager->forceUpdateFailure = true;
    ASSERT_TRUE(view->triggerEditRequested("new_front", "new_back"));

    auto list = deckManager->getList("MyDeck");
    ASSERT_NE(list, nullptr);
    auto existing = list->getCard("card-to-edit");
    ASSERT_NE(existing, nullptr);
    EXPECT_EQ(existing->text_front, "old_front");
    EXPECT_EQ(existing->text_back, "old_back");
    EXPECT_EQ(view->showCardCount, 1); // only initial handleStep show
}

TEST_F(StudySessionPresenterTest, OnDeleteRequestedWhenRemoveFailsKeepsCard) {
    auto card = std::make_shared<core::Flashcard>();
    card->card_id = "card-to-delete";
    deckManager->addList("MyDeck", "path/to/deck.json", {card});

    core::SessionConfig config;
    presenter->start("MyDeck", "path/to/deck.json", config);
    presenter->handleStep();

    deckManager->forceRemoveFailure = true;
    ASSERT_TRUE(view->triggerDeleteRequested());

    auto list = deckManager->getList("MyDeck");
    ASSERT_NE(list, nullptr);
    EXPECT_NE(list->getCard("card-to-delete"), nullptr);
}

TEST_F(StudySessionPresenterTest, OnResetAllRequestedResetsCardStatesToNewAndSaves) {
    auto card1 = std::make_shared<core::Flashcard>();
    card1->card_id = "card-1";
    card1->state_Front_to_Back = core::CardState::Known;
    card1->state_Back_to_Front = core::CardState::Mastered;

    auto card2 = std::make_shared<core::Flashcard>();
    card2->card_id = "card-2";
    card2->state_Front_to_Back = core::CardState::Mastered;
    card2->state_Back_to_Front = core::CardState::Known;

    deckManager->addList("MyDeck", "path/to/deck.json", {card1, card2});

    core::SessionConfig config;
    presenter->start("MyDeck", "path/to/deck.json", config);
    presenter->handleStep();

    ASSERT_TRUE(view->triggerResetAllRequested());

    auto list = deckManager->getList("MyDeck");
    ASSERT_NE(list, nullptr);
    auto cards = list->getAllCards();
    ASSERT_EQ(cards.size(), 2U);
    for (const auto& card : cards) {
        ASSERT_NE(card, nullptr);
        EXPECT_EQ(card->state_Front_to_Back, core::CardState::New);
        EXPECT_EQ(card->state_Back_to_Front, core::CardState::New);
    }

    ASSERT_FALSE(deckManager->savedLists.empty());
    EXPECT_EQ(deckManager->savedLists.back(), "MyDeck");
}

} // namespace app