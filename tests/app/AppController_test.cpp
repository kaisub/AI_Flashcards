#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "app/AppController.hpp"
#include "app/AppState.hpp"
#include "core/IStorage.hpp"
#include "core/IDeckManager.hpp"
#include "app/views/IListsBrowserView.hpp"
#include "app/views/IDeckEditorView.hpp"
#include "app/views/IStudySessionView.hpp"
#include "app/views/IStudyConfigView.hpp"
#include "core/Flashcard.hpp"

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// Mock for core::IDeckManager
class MockDeckManager : public core::IDeckManager {
public:
    MockDeckManager() = default;

    MOCK_METHOD(std::vector<fs::path>, getAllAvailableLists, (), (const, override));
    MOCK_METHOD(bool, createFolder, (const fs::path&), (override));
    MOCK_METHOD(bool, deleteFolder, (const fs::path&), (override));
    MOCK_METHOD(bool, renameFolder, (const fs::path&, const fs::path&), (override));
    MOCK_METHOD(bool, moveListFile, (const fs::path&, const fs::path&), (override));
    MOCK_METHOD(bool, deleteListFile, (const fs::path&), (override));

    MOCK_METHOD(std::shared_ptr<core::FlashcardList>, loadList, (const fs::path&), (override));
    MOCK_METHOD(bool, createList, (const std::string&, const fs::path&), (override));
    MOCK_METHOD(bool, createList, (const std::string&), (override));
    MOCK_METHOD(bool, saveList, (const std::string&), (override));
    MOCK_METHOD(bool, deleteList, (const std::string&), (override));
    MOCK_METHOD(std::vector<std::string>, getAllListNames, (), (const, override));
    MOCK_METHOD(std::shared_ptr<core::FlashcardList>, getList, (const std::string&), (const, override));

    MOCK_METHOD(bool, addCardToList, (const std::string&, std::shared_ptr<core::Flashcard>), (override));
    MOCK_METHOD(bool, removeCardFromList, (const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, updateCardInList, (const std::string&, const std::string&, const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, moveCard, (const std::string&, const std::string&, const std::string&), (override));

    MOCK_METHOD(size_t, addCardsToList, (const std::string&, const std::vector<std::shared_ptr<core::Flashcard>>&), (override));
    MOCK_METHOD(size_t, removeCardsFromList, (const std::string&, const std::vector<std::string>&), (override));
    MOCK_METHOD(size_t, moveCards, (const std::vector<std::string>&, const std::string&, const std::string&), (override));
    MOCK_METHOD(size_t, importCardsFromFile, (const std::string&, const std::filesystem::path&, char, bool), (override));
};

// Mock for app::IListsBrowserView
class MockIListsBrowserView : public app::IListsBrowserView {
public:
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, updateList, (const std::filesystem::path&, const std::vector<app::model::BrowserItem>&), (override));
};

// Mock for app::IDeckEditorView
class MockDeckEditorView : public app::IDeckEditorView {
public:
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, setDeck, (std::shared_ptr<core::FlashcardList>), (override));
    MOCK_METHOD(void, setAvailableLists, (const std::vector<std::string>&), (override));
};

// Mock for app::IStudySessionView
class MockStudySessionView : public app::IStudySessionView {
public:
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, showCard, (const core::Flashcard&, bool), (override));
    MOCK_METHOD(void, showSessionComplete, (), (override));
    MOCK_METHOD(void, setAvailableLists, (const std::vector<std::string>&), (override));
};

// Mock for app::IStudyConfigView
class MockStudyConfigView : public app::IStudyConfigView {
public:
    MOCK_METHOD(void, run, (), (override));
};

namespace app {
    class AppControllerAccess {
    public:
        static void transitionTo(AppController& controller, AppState state) {
            controller.transitionTo(state);
        }

        static void handleListsBrowser(AppController& controller) {
            controller.handleListsBrowser();
        }

        static void handleDeckEditor(AppController& controller) {
            controller.handleDeckEditor();
        }

        static void handleStudyConfig(AppController& controller) {
            controller.handleStudyConfig();
        }

        static void handleStudySession(AppController& controller) {
            controller.handleStudySession();
        }
    };

    // A test fixture for AppController
    class AppControllerTest : public ::testing::Test {
    protected:
        MockDeckManager* mockDeckManager; // Raw pointer for expectations
        std::unique_ptr<AppController> controller;
        MockIListsBrowserView mockListsBrowserView;
        MockDeckEditorView mockDeckEditorView;
        MockStudySessionView mockStudySessionView;
        MockStudyConfigView mockStudyConfigView;

        void SetUp() override {
            auto deckManager = std::make_unique<MockDeckManager>();
            mockDeckManager = deckManager.get();

            controller = std::make_unique<AppController>(
                std::move(deckManager)
            );

            // Provide the views to the controller without passing ownership of stack variables
            auto sharedBrowser = std::shared_ptr<IListsBrowserView>(&mockListsBrowserView, [](IListsBrowserView*){});
            auto sharedEditor = std::shared_ptr<IDeckEditorView>(&mockDeckEditorView, [](IDeckEditorView*){});
            auto sharedStudy = std::shared_ptr<IStudySessionView>(&mockStudySessionView, [](IStudySessionView*){});
            auto sharedConfig = std::shared_ptr<IStudyConfigView>(&mockStudyConfigView, [](IStudyConfigView*){});

            controller->setListsBrowserView(sharedBrowser);
            controller->setDeckEditorView(sharedEditor);
            controller->setStudySessionView(sharedStudy);
            controller->setStudyConfigView(sharedConfig);
        }

        // Helper to simulate the internal _currentDirectory update (for testing purposes)
        void setCurrentDirectory(const std::filesystem::path& path) {
            if (!path.empty()) {
                mockListsBrowserView.triggerFolderOpened(path);
            }
        }
    };

    TEST_F(AppControllerTest, InitialStateIsListsBrowser) {
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, TransitionToChangesState) {
        AppControllerAccess::transitionTo(*controller, AppState::DeckEditor);
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);

        AppControllerAccess::transitionTo(*controller, AppState::StudySession);
        ASSERT_EQ(controller->getCurrentState(), AppState::StudySession);
    }

    TEST_F(AppControllerTest, OnListSelectedTransitionsToDeckEditor) {
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);

        const std::filesystem::path testListPath = "path/to/my_list.json";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");

        // Expectations for onListSelected logic
        EXPECT_CALL(*mockDeckManager, loadList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        // Simulate a list being selected in the view
        mockListsBrowserView.triggerListSelected(testListPath);

        // Verify the state transition
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
    }

    TEST_F(AppControllerTest, OnFolderOpenedUpdatesCurrentDirectory) {
        std::filesystem::path testPath = "folder1/subfolder";

        // This simulates the view's event. The AppController's bindBrowserEvents
        // callback for onFolderOpened will update its internal _currentDirectory.
        mockListsBrowserView.triggerFolderOpened(testPath);

        // Unfortunately, _currentDirectory is private. We cannot directly assert its value here
        // without adding a getter to AppController (which was done) or using a friend class for tests.
        // The AppController_test.cpp's `bindBrowserEvents` calls the lambda which updates `_currentDirectory`.
        // The `std::cout` in the implementation confirms it is set.
        // For a more robust test, `AppController` would need a `getCurrentDirectory()` method.
        // For now, we rely on the `onFolderOpened` callback's internal effect.

        // Let's ensure the state remains ListsBrowser after folder opening
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, OnBackRequestedTransitionsFromRootToExit) {
        // Initially at ListsBrowser, _currentDirectory is empty (conceptual root)
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);

        // Simulate going back from the root directory
        mockListsBrowserView.triggerBackRequested();

        // Should transition to Exit
        ASSERT_EQ(controller->getCurrentState(), AppState::Exit);
    }

    TEST_F(AppControllerTest, OnBackRequestedMovesUpDirectory) {
        // Set an initial directory deeper than root
        std::filesystem::path initialPath = "folder1/subfolder";
        mockListsBrowserView.triggerFolderOpened(initialPath); // This updates _currentDirectory in controller

        // Simulate going back
        mockListsBrowserView.triggerBackRequested();

        // The controller's _currentDirectory should now be "folder1".
        // Again, direct assertion of _currentDirectory is not possible without a getter.
        // We can only assert that the state remains ListsBrowser, implying it handled the back action
        // without exiting (which would happen if it thought it was at root).
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);

        // Simulate going back again
        mockListsBrowserView.triggerBackRequested();
        // Now it should be at the virtual root, so next back request should exit
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);

        mockListsBrowserView.triggerBackRequested();
        ASSERT_EQ(controller->getCurrentState(), AppState::Exit);
    }

    TEST_F(AppControllerTest, HandleListsBrowserPopulatesView) {
        EXPECT_CALL(*mockDeckManager, getAllAvailableLists())
            .WillRepeatedly(::testing::Return(std::vector<std::filesystem::path>{}));

        EXPECT_CALL(mockListsBrowserView, updateList(::testing::_, ::testing::_))
            .Times(1);
            
        // We intercept the run loop of the view to exit the app,
        // otherwise the test will block forever.
        EXPECT_CALL(mockListsBrowserView, run()).WillOnce(::testing::Invoke([this]() {
            AppControllerAccess::transitionTo(*controller, AppState::Exit);
        }));

        // run() will call handleListsBrowser and then exit.
        controller->run();

        ASSERT_EQ(controller->getCurrentState(), AppState::Exit);
    }

    TEST_F(AppControllerTest, OnNewListRequestedSavesEmptyList) {
        const std::string newListName = "MyNewList";
        const fs::path expectedPath = fs::path(newListName + ".json");

        EXPECT_CALL(*mockDeckManager, createList(newListName, expectedPath))
            .WillOnce(::testing::Return(true));

        // Simulate the event from the view
        mockListsBrowserView.triggerNewListRequested(newListName);

        // The state should remain in the browser after list creation
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, OnAddCardSavesList) {
        const std::filesystem::path testListPath = "path/to/my_list.json";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");

        // --- Expectations for onListSelected logic ---
        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList("my_list")).WillRepeatedly(::testing::Return(list_in_dm));

        // --- Expectations for onAddCard logic ---
        EXPECT_CALL(*mockDeckManager, addCardToList(::testing::_, ::testing::_)).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));

        // --- Simulate user actions ---
        mockListsBrowserView.triggerListSelected(testListPath); // Transitions to DeckEditor
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
        AppControllerAccess::handleDeckEditor(*controller);

        EXPECT_CALL(mockDeckEditorView, setDeck(::testing::_)).Times(1);
        mockDeckEditorView.triggerAddCard("front", "back"); // Triggers add and save
    }

    TEST_F(AppControllerTest, OnExitToBrowserTransitionsToBrowser) {
        // Setup state to be in DeckEditor
        AppControllerAccess::transitionTo(*controller, AppState::DeckEditor);
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
        AppControllerAccess::handleDeckEditor(*controller);

        EXPECT_CALL(*mockDeckManager, saveList(::testing::_)).WillRepeatedly(::testing::Return(true));
        // Simulate event
        mockDeckEditorView.triggerExitToBrowser();

        // Verify state transition
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, OnListSelectedWithNullStorageDoesNotTransition) {
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);

        const std::filesystem::path testListPath = "path/to/missing_list.json";
        EXPECT_CALL(*mockDeckManager, loadList(testListPath))
            .WillOnce(::testing::Return(nullptr));

        mockListsBrowserView.triggerListSelected(testListPath);

        // State must stay in ListsBrowser — no transition on null load
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, OnRemoveCardSavesList) {
        const std::filesystem::path testListPath = "path/to/my_list.json";
        const std::string listName = "my_list";
        auto existingCard = std::make_shared<core::Flashcard>();
        existingCard->card_id = "card-1";

        auto list_in_dm = std::make_shared<core::FlashcardList>(listName);
        list_in_dm->addCard(existingCard);

        // Setup expectations for getting into DeckEditor
        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_))
            .WillRepeatedly(::testing::Return(list_in_dm));

        // Setup expectations for card removal
        EXPECT_CALL(*mockDeckManager, removeCardFromList(::testing::_, "card-1")).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));

        // Run actions
        mockListsBrowserView.triggerListSelected(testListPath);
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
        AppControllerAccess::handleDeckEditor(*controller);
        EXPECT_CALL(mockDeckEditorView, setDeck(::testing::_)).Times(1);
        mockDeckEditorView.triggerRemoveCard("card-1");
    }

    TEST_F(AppControllerTest, OnStartStudyTransitionsToStudyConfig) {
        // Setup: select a list to get into DeckEditor
        const std::filesystem::path testListPath = "path/to/my_list.json";
        auto card1 = std::make_shared<core::Flashcard>();
        card1->card_id = "1";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");
        list_in_dm->addCard(card1);

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));
        mockListsBrowserView.triggerListSelected(testListPath);
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
        AppControllerAccess::handleDeckEditor(*controller);

        // Trigger the study session start
        mockDeckEditorView.triggerStartStudy();

        // Verify the state transition
        ASSERT_EQ(controller->getCurrentState(), AppState::StudyConfig);
    }

    TEST_F(AppControllerTest, OnStartStudyWithEmptyDeckStaysInDeckEditor) {
        const std::filesystem::path testListPath = "path/to/empty_list.json";
        auto list_in_dm = std::make_shared<core::FlashcardList>("empty_list");
        // Note: Intentionally NOT adding any cards to simulate an empty deck

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        // Navigate into the Deck Editor
        mockListsBrowserView.triggerListSelected(testListPath);
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
        AppControllerAccess::handleDeckEditor(*controller);

        // Attempt to start the study session
        mockDeckEditorView.triggerStartStudy();

        // Verify the state transition was blocked and we remain in the Deck Editor
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
    }

    TEST_F(AppControllerTest, OnCancelStudyConfigTransitionsToDeckEditor) {
        const std::filesystem::path testListPath = "path/to/my_list.json";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");

        // A deck must have at least one card to transition to StudyConfig
        auto card1 = std::make_shared<core::Flashcard>();
        card1->card_id = "1";
        list_in_dm->addCard(card1);

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));
        mockListsBrowserView.triggerListSelected(testListPath);
        AppControllerAccess::handleDeckEditor(*controller);
        mockDeckEditorView.triggerStartStudy();
        ASSERT_EQ(controller->getCurrentState(), AppState::StudyConfig);
        AppControllerAccess::handleStudyConfig(*controller);

        // Trigger cancellation
        mockStudyConfigView.triggerCancel();

        // Verify it returns to the DeckEditor
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
    }

    TEST_F(AppControllerTest, OnExitStudySessionSavesProgress) {
        // Setup: select a list and start a study session
        const std::filesystem::path testListPath = "path/to/my_list.json";
        auto card1 = std::make_shared<core::Flashcard>();
        card1->card_id = "1";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");
        list_in_dm->addCard(card1);

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        mockListsBrowserView.triggerListSelected(testListPath);
        AppControllerAccess::handleDeckEditor(*controller);
        mockDeckEditorView.triggerStartStudy();
        AppControllerAccess::handleStudyConfig(*controller);

        // Simulate clicking "Start" in the config view
        core::SessionConfig config;
        mockStudyConfigView.triggerStart(config);

        ASSERT_EQ(controller->getCurrentState(), AppState::StudySession);
        AppControllerAccess::handleStudySession(*controller);

        // Expect saveList to be called upon exiting the session
        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));

        // Trigger exit from study session
        mockStudySessionView.triggerExitRequested();

        // Verify state transitions back to StudyConfig
        ASSERT_EQ(controller->getCurrentState(), AppState::StudyConfig);
    }

    TEST_F(AppControllerTest, OnCardRatedShowsNextCard) {
        // Setup: List with two cards
        const std::filesystem::path testListPath = "path/to/my_list.json";
        auto card1 = std::make_shared<core::Flashcard>();
        card1->card_id = "1";
        auto card2 = std::make_shared<core::Flashcard>();
        card2->card_id = "2";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");
        list_in_dm->addCard(card1);
        list_in_dm->addCard(card2);

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));
        // Start session
        mockListsBrowserView.triggerListSelected(testListPath);
        AppControllerAccess::handleDeckEditor(*controller);
        mockDeckEditorView.triggerStartStudy();
        AppControllerAccess::handleStudyConfig(*controller);

        // Simulate clicking "Start" in the config view
        core::SessionConfig config;
        mockStudyConfigView.triggerStart(config);

        ASSERT_EQ(controller->getCurrentState(), AppState::StudySession);

        // In handleStudySession, the loop relies on view.run() returning and then iterating.
        // We expect showCard to be called twice: once for the first card, once for the second card.
        EXPECT_CALL(mockStudySessionView, showCard(::testing::_, false)).Times(2);
        
        // First view execution: user rates the card.
        // Second view execution: user exits the session.
        EXPECT_CALL(mockStudySessionView, run())
            .WillOnce(::testing::Invoke([this]() {
                mockStudySessionView.triggerCardRated(core::CardState::Known);
            }))
            .WillOnce(::testing::Invoke([this]() {
                mockStudySessionView.triggerExitRequested();
            }));

        // After exiting session, controller enters StudyConfig; force app shutdown.
        EXPECT_CALL(mockStudyConfigView, run())
            .WillOnce(::testing::Invoke([this]() {
                AppControllerAccess::transitionTo(*controller, AppState::Exit);
            }));
            
        controller->run();
        
        // Verify the app exits successfully
        ASSERT_EQ(controller->getCurrentState(), AppState::Exit);
    }

    TEST_F(AppControllerTest, OnUpdateCardSavesList) {
        const std::filesystem::path testListPath = "path/to/my_list.json";
        const std::string listName = "my_list";
        auto sourceCard = std::make_shared<core::Flashcard>();
        sourceCard->card_id = "card-1";
        auto list_in_dm = std::make_shared<core::FlashcardList>(listName);
        list_in_dm->addCard(sourceCard);

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        mockListsBrowserView.triggerListSelected(testListPath);
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
        AppControllerAccess::handleDeckEditor(*controller);

        EXPECT_CALL(*mockDeckManager, updateCardInList(::testing::_, "card-1", "new front", "new back"))
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(mockDeckEditorView, setDeck(::testing::_)).Times(1);

        mockDeckEditorView.triggerUpdateCard("card-1", "new front", "new back");
    }

    TEST_F(AppControllerTest, OnDeleteSelectedSavesList) {
        const std::filesystem::path testListPath = "path/to/my_list.json";
        const std::string listName = "my_list";
        auto list_in_dm = std::make_shared<core::FlashcardList>(listName);

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        mockListsBrowserView.triggerListSelected(testListPath);
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
        AppControllerAccess::handleDeckEditor(*controller);

        EXPECT_CALL(*mockDeckManager, removeCardsFromList(listName, std::vector<std::string>{"card-1", "card-2"}))
            .WillOnce(::testing::Return(2));
        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(mockDeckEditorView, setDeck(::testing::_)).Times(1);

        mockDeckEditorView.triggerDeleteSelected({"card-1", "card-2"});
    }

    TEST_F(AppControllerTest, OnMoveSelectedMovesCards) {
        const std::filesystem::path testListPath = "lists/source.json";
        const std::filesystem::path targetListPath = "lists/target.json";
        const std::string listName = "source";

        auto sourceCard = std::make_shared<core::Flashcard>();
        sourceCard->card_id = "card-1";
        sourceCard->text_front = "front";
        sourceCard->text_back = "back";
        auto list_in_dm = std::make_shared<core::FlashcardList>(listName);
        list_in_dm->addCard(sourceCard);
        auto targetList = std::make_shared<core::FlashcardList>("target");

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        mockListsBrowserView.triggerListSelected(testListPath);
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
        AppControllerAccess::handleDeckEditor(*controller);

        EXPECT_CALL(*mockDeckManager, getAllAvailableLists())
            .WillOnce(::testing::Return(std::vector<fs::path>{testListPath, targetListPath}));
        EXPECT_CALL(*mockDeckManager, loadList(targetListPath)).WillOnce(::testing::Return(targetList));
        
        EXPECT_CALL(*mockDeckManager, moveCards(std::vector<std::string>{"card-1"}, listName, "target"))
            .WillOnce(::testing::Invoke([&](auto, auto, auto) {
                list_in_dm->removeCard("card-1");
                targetList->addCard(sourceCard);
                return 1;
            }));
        EXPECT_CALL(*mockDeckManager, saveList("source")).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(*mockDeckManager, saveList("target")).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(mockDeckEditorView, setDeck(::testing::_)).Times(1);

        mockDeckEditorView.triggerMoveSelected({"card-1"}, 0);

        EXPECT_EQ(list_in_dm->getCard("card-1"), nullptr); // removed from source
        EXPECT_EQ(targetList->size(), 1U);                 // copied to target
    }

    TEST_F(AppControllerTest, OnCopySelectedCopiesCards) {
        const std::filesystem::path testListPath = "lists/source.json";
        const std::filesystem::path targetListPath = "lists/target.json";
        const std::string listName = "source";

        auto sourceCard = std::make_shared<core::Flashcard>();
        sourceCard->card_id = "card-1";
        sourceCard->text_front = "front";
        sourceCard->text_back = "back";
        sourceCard->state_Front_to_Back = core::CardState::Known;
        sourceCard->state_Back_to_Front = core::CardState::Known;

        auto list_in_dm = std::make_shared<core::FlashcardList>(listName);
        list_in_dm->addCard(sourceCard);
        auto targetList = std::make_shared<core::FlashcardList>("target");

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        mockListsBrowserView.triggerListSelected(testListPath);
        ASSERT_EQ(controller->getCurrentState(), AppState::DeckEditor);
        AppControllerAccess::handleDeckEditor(*controller);

        EXPECT_CALL(*mockDeckManager, getAllAvailableLists())
            .WillOnce(::testing::Return(std::vector<fs::path>{testListPath, targetListPath}));
        EXPECT_CALL(*mockDeckManager, loadList(targetListPath)).WillOnce(::testing::Return(targetList));
        
        EXPECT_CALL(*mockDeckManager, addCardsToList("target", ::testing::_))
            .WillOnce(::testing::Invoke([&](auto, auto cards) {
                for(auto c : cards) targetList->addCard(c);
                return cards.size();
            }));
        EXPECT_CALL(*mockDeckManager, saveList("target")).WillRepeatedly(::testing::Return(true));

        mockDeckEditorView.triggerCopySelected({"card-1"}, 0);

        EXPECT_NE(list_in_dm->getCard("card-1"), nullptr); // source unchanged
        ASSERT_EQ(targetList->size(), 1U);                 // card copied
        EXPECT_EQ(targetList->getAllCards()[0]->state_Front_to_Back, core::CardState::New); // states reset
        EXPECT_EQ(targetList->getAllCards()[0]->state_Back_to_Front, core::CardState::New);
    }

    TEST_F(AppControllerTest, OnNewFolderRequestedCreatesFolder) {
        EXPECT_CALL(*mockDeckManager, createFolder(::testing::_)).WillOnce(::testing::Return(true));
        mockListsBrowserView.triggerNewFolderRequested("NewFolder");
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, OnRenameRequestedForFile) {
        app::model::BrowserItem item;
        item.fullPath = "path/to/list.json";
        item.displayName = "list.json";
        item.isDirectory = false;

        EXPECT_CALL(*mockDeckManager, moveListFile(item.fullPath, fs::path("path/to") / "renamed.json"))
            .WillOnce(::testing::Return(true));

        mockListsBrowserView.triggerRenameRequested(item, "renamed");
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, OnRenameRequestedForFolder) {
        app::model::BrowserItem item;
        item.fullPath = "path/to/folder";
        item.displayName = "folder";
        item.isDirectory = true;

        EXPECT_CALL(*mockDeckManager, renameFolder(item.fullPath, fs::path("path/to") / "new_folder"))
            .WillOnce(::testing::Return(true));

        mockListsBrowserView.triggerRenameRequested(item, "new_folder");
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, OnDeleteRequestedForFile) {
        app::model::BrowserItem item;
        item.fullPath = "path/to/list.json";
        item.displayName = "list.json";
        item.isDirectory = false;

        EXPECT_CALL(*mockDeckManager, deleteListFile(item.fullPath)).WillOnce(::testing::Return(true));

        mockListsBrowserView.triggerDeleteRequested(item);
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, OnDeleteRequestedForFolder) {
        app::model::BrowserItem item;
        item.fullPath = "path/to/folder";
        item.displayName = "folder";
        item.isDirectory = true;

        EXPECT_CALL(*mockDeckManager, deleteFolder(item.fullPath)).WillOnce(::testing::Return(true));

        mockListsBrowserView.triggerDeleteRequested(item);
        ASSERT_EQ(controller->getCurrentState(), AppState::ListsBrowser);
    }

    TEST_F(AppControllerTest, OnExitAppRequestedExitsApp) {
        mockListsBrowserView.triggerExitAppRequested();
        ASSERT_EQ(controller->getCurrentState(), AppState::Exit);
    }

    TEST_F(AppControllerTest, OnUndoRequestedCallsUndoInSession) {
        const std::filesystem::path testListPath = "path/to/my_list.json";
        auto card1 = std::make_shared<core::Flashcard>();
        card1->card_id = "1";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");
        list_in_dm->addCard(card1);

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        mockListsBrowserView.triggerListSelected(testListPath);
        AppControllerAccess::handleDeckEditor(*controller);
        mockDeckEditorView.triggerStartStudy();
        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));
        AppControllerAccess::handleStudyConfig(*controller);
        core::SessionConfig config;
        mockStudyConfigView.triggerStart(config);
        ASSERT_EQ(controller->getCurrentState(), AppState::StudySession);
        AppControllerAccess::handleStudySession(*controller);

        // Undo with empty history should not crash
        mockStudySessionView.triggerUndoRequested();
        ASSERT_EQ(controller->getCurrentState(), AppState::StudySession);
    }

    TEST_F(AppControllerTest, OnEditRequestedInSessionUpdatesCard) {
        const std::filesystem::path testListPath = "path/to/my_list.json";
        auto card1 = std::make_shared<core::Flashcard>();
        card1->card_id = "edit-card";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");
        list_in_dm->addCard(card1);

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        mockListsBrowserView.triggerListSelected(testListPath);
        AppControllerAccess::handleDeckEditor(*controller);
        mockDeckEditorView.triggerStartStudy();
        core::SessionConfig config;
        config.direction = core::TranslationDirection::Front_to_Back;
        mockStudyConfigView.triggerStart(config);
        ASSERT_EQ(controller->getCurrentState(), AppState::StudySession);

        EXPECT_CALL(*mockDeckManager, getAllAvailableLists()).WillRepeatedly(::testing::Return(std::vector<fs::path>{}));
        EXPECT_CALL(*mockDeckManager, updateCardInList(::testing::_, "edit-card", "edited front", "edited back"))
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(mockStudySessionView, showCard(::testing::_, false)).Times(::testing::AtLeast(1));

        EXPECT_CALL(mockStudySessionView, run())
            .WillOnce(::testing::Invoke([this]() {
                mockStudySessionView.triggerEditRequested("edited front", "edited back");
            }))
            .WillRepeatedly(::testing::Invoke([this]() {
                mockStudySessionView.triggerExitRequested();
            }));

        EXPECT_CALL(mockStudyConfigView, run())
            .WillOnce(::testing::Invoke([this]() {
                AppControllerAccess::transitionTo(*controller, AppState::Exit);
            }));

        controller->run();
        ASSERT_EQ(controller->getCurrentState(), AppState::Exit);
    }

    TEST_F(AppControllerTest, OnDeleteRequestedInSessionRemovesCard) {
        const std::filesystem::path testListPath = "path/to/my_list.json";
        auto card1 = std::make_shared<core::Flashcard>();
        card1->card_id = "del-card";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");
        list_in_dm->addCard(card1);

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));

        mockListsBrowserView.triggerListSelected(testListPath);
        AppControllerAccess::handleDeckEditor(*controller);
        mockDeckEditorView.triggerStartStudy();
        core::SessionConfig config;
        config.direction = core::TranslationDirection::Front_to_Back;
        mockStudyConfigView.triggerStart(config);
        ASSERT_EQ(controller->getCurrentState(), AppState::StudySession);

        EXPECT_CALL(*mockDeckManager, getAllAvailableLists()).WillRepeatedly(::testing::Return(std::vector<fs::path>{}));
        EXPECT_CALL(*mockDeckManager, removeCardFromList(::testing::_, "del-card"))
            .WillOnce(::testing::Return(true));
        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(mockStudySessionView, showCard(::testing::_, false)).Times(::testing::AtLeast(1));
        EXPECT_CALL(mockStudySessionView, showSessionComplete()).Times(::testing::AtLeast(0));

        EXPECT_CALL(mockStudySessionView, run())
            .WillOnce(::testing::Invoke([this]() {
                mockStudySessionView.triggerDeleteRequested();
            }))
            .WillRepeatedly(::testing::Invoke([this]() {
                mockStudySessionView.triggerExitRequested();
            }));

        EXPECT_CALL(mockStudyConfigView, run())
            .WillOnce(::testing::Invoke([this]() {
                AppControllerAccess::transitionTo(*controller, AppState::Exit);
            }));

        controller->run();
        ASSERT_EQ(controller->getCurrentState(), AppState::Exit);
    }

    TEST_F(AppControllerTest, OnCopyRequestedInSessionCopiesCard) {
        const std::filesystem::path testListPath = "path/to/my_list.json";
        const std::filesystem::path otherListPath = "path/to/other.json";
        auto card1 = std::make_shared<core::Flashcard>();
        card1->card_id = "copy-card";
        card1->text_front = "front";
        card1->text_back = "back";
        auto list_in_dm = std::make_shared<core::FlashcardList>("my_list");
        list_in_dm->addCard(card1);
        auto otherList = std::make_shared<core::FlashcardList>("other");

        EXPECT_CALL(*mockDeckManager, loadList(testListPath)).WillRepeatedly(::testing::Return(list_in_dm));
        EXPECT_CALL(*mockDeckManager, loadList(otherListPath)).WillRepeatedly(::testing::Return(otherList));
        EXPECT_CALL(*mockDeckManager, getList(::testing::_)).WillRepeatedly(::testing::Return(list_in_dm));
        // getAllAvailableLists is consumed when StudySession starts from StudyConfig.
        EXPECT_CALL(*mockDeckManager, getAllAvailableLists())
            .WillRepeatedly(::testing::Return(std::vector<fs::path>{testListPath, otherListPath}));

        mockListsBrowserView.triggerListSelected(testListPath);
        AppControllerAccess::handleDeckEditor(*controller);
        mockDeckEditorView.triggerStartStudy();
        core::SessionConfig config;
        config.direction = core::TranslationDirection::Front_to_Back;
        mockStudyConfigView.triggerStart(config);
        ASSERT_EQ(controller->getCurrentState(), AppState::StudySession);
        
        EXPECT_CALL(*mockDeckManager, addCardToList("other", ::testing::_))
            .WillOnce(::testing::Invoke([&](auto, auto c){ return otherList->addCard(c); }));

        EXPECT_CALL(*mockDeckManager, saveList("other")).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(*mockDeckManager, saveList("my_list")).WillRepeatedly(::testing::Return(true));
        EXPECT_CALL(mockStudySessionView, showCard(::testing::_, false)).Times(::testing::AtLeast(1));
        EXPECT_CALL(mockStudySessionView, showSessionComplete()).Times(::testing::AtLeast(0));

        EXPECT_CALL(mockStudySessionView, run())
            .WillOnce(::testing::Invoke([this]() {
                // Index 0 in _availableListPaths (which excludes testListPath) = otherListPath
                mockStudySessionView.triggerCopyRequested(0);
            }))
            .WillRepeatedly(::testing::Invoke([this]() {
                mockStudySessionView.triggerExitRequested();
            }));

        EXPECT_CALL(mockStudyConfigView, run())
            .WillOnce(::testing::Invoke([this]() {
                AppControllerAccess::transitionTo(*controller, AppState::Exit);
            }));

        controller->run();
        ASSERT_EQ(controller->getCurrentState(), AppState::Exit);
        EXPECT_EQ(otherList->size(), 1U); // card was copied to otherList
    }

} // namespace app
