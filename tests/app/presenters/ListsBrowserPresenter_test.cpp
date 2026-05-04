#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "app/presenters/ListsBrowserPresenter.hpp"
#include "app/views/IListsBrowserView.hpp"
#include "core/IDeckManager.hpp"
#include <filesystem>

namespace app {

// Mocks copied from AppController_test.cpp for isolation
class MockIListsBrowserView : public IListsBrowserView {
public:
    MOCK_METHOD(void, run, (), (override));
    MOCK_METHOD(void, updateList, (const std::filesystem::path&, const std::vector<app::model::BrowserItem>&), (override));
};

class MockDeckManager : public core::IDeckManager {
public:
    MockDeckManager() = default;
    MOCK_METHOD(std::vector<std::filesystem::path>, getAllAvailableLists, (), (const, override));
    MOCK_METHOD(bool, createList, (const std::string&, const std::filesystem::path&), (override));
    MOCK_METHOD(bool, createFolder, (const std::filesystem::path&), (override));
    MOCK_METHOD(bool, renameFolder, (const std::filesystem::path&, const std::filesystem::path&), (override));
    MOCK_METHOD(bool, moveListFile, (const std::filesystem::path&, const std::filesystem::path&), (override));
    MOCK_METHOD(bool, deleteFolder, (const std::filesystem::path&), (override));
    MOCK_METHOD(bool, deleteListFile, (const std::filesystem::path&), (override));

    MOCK_METHOD(std::shared_ptr<core::FlashcardList>, loadList, (const std::filesystem::path&), (override));
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

class ListsBrowserPresenterTest : public ::testing::Test {
protected:
    std::shared_ptr<MockIListsBrowserView> mockView;
    std::unique_ptr<MockDeckManager> ownedDeckManager;
    MockDeckManager* mockDeckManager;
    std::unique_ptr<presenters::ListsBrowserPresenter> presenter;

    void SetUp() override {
        mockView = std::make_shared<MockIListsBrowserView>();
        ownedDeckManager = std::make_unique<MockDeckManager>();
        mockDeckManager = ownedDeckManager.get();
        presenter = std::make_unique<presenters::ListsBrowserPresenter>(mockView, mockDeckManager);
    }
};

TEST_F(ListsBrowserPresenterTest, StartPopulatesViewWithSortedItems) {
    std::vector<std::filesystem::path> unsortedPaths = {"b.json", "a.json", "dir1"};
    EXPECT_CALL(*mockDeckManager, getAllAvailableLists()).WillOnce(::testing::Return(unsortedPaths));

    // Expect updateList to be called with sorted items (dir first, then alpha)
    EXPECT_CALL(*mockView, updateList(::testing::_, ::testing::Truly([](const std::vector<app::model::BrowserItem>& items) {
        return items.size() == 3U &&
               items[0].displayName == "dir1" && items[0].isDirectory &&
               items[1].displayName == "a.json" && !items[1].isDirectory &&
               items[2].displayName == "b.json" && !items[2].isDirectory;
    }))).Times(1);

    EXPECT_CALL(*mockView, run()).Times(1);

    presenter->start();
}

TEST_F(ListsBrowserPresenterTest, OnNewListRequestedEnsuresJsonExtension) {
    EXPECT_CALL(*mockDeckManager, createList("NewList", std::filesystem::path("NewList.json"))).WillOnce(::testing::Return(true));
    ASSERT_TRUE(mockView->triggerNewListRequested("NewList"));

    EXPECT_CALL(*mockDeckManager, createList("Existing.json", std::filesystem::path("Existing.json"))).WillOnce(::testing::Return(true));
    ASSERT_TRUE(mockView->triggerNewListRequested("Existing.json"));
}

TEST_F(ListsBrowserPresenterTest, OnRenameRequestedForFileEnsuresJsonExtension) {
    app::model::BrowserItem item;
    item.isDirectory = false;

    EXPECT_CALL(*mockDeckManager, moveListFile(::testing::_, std::filesystem::path("renamed.json"))).WillOnce(::testing::Return(true));
    ASSERT_TRUE(mockView->triggerRenameRequested(item, "renamed"));

    EXPECT_CALL(*mockDeckManager, moveListFile(::testing::_, std::filesystem::path("renamed.json"))).WillOnce(::testing::Return(true));
    ASSERT_TRUE(mockView->triggerRenameRequested(item, "renamed.json"));
}

TEST_F(ListsBrowserPresenterTest, OnBackRequestedFromRootFiresExitCallback) {
    bool exitRequested = false;
    presenter->onExitAppRequested = [&]() { exitRequested = true; };

    ASSERT_TRUE(mockView->triggerBackRequested());

    EXPECT_TRUE(exitRequested);
}

TEST_F(ListsBrowserPresenterTest, OnBackRequestedFromNestedDirectoryDoesNotExit) {
    bool exitRequested = false;
    presenter->onExitAppRequested = [&]() { exitRequested = true; };

    ASSERT_TRUE(mockView->triggerFolderOpened("dir/subdir"));
    ASSERT_TRUE(mockView->triggerBackRequested());

    EXPECT_FALSE(exitRequested);
}

TEST_F(ListsBrowserPresenterTest, StartFiltersOutsideCurrentDirectoryEntries) {
    ASSERT_TRUE(mockView->triggerFolderOpened("root"));

    std::vector<std::filesystem::path> paths = {
        "root/inside.json",
        "other/outside.json"
    };

    EXPECT_CALL(*mockDeckManager, getAllAvailableLists()).WillOnce(::testing::Return(paths));
    EXPECT_CALL(*mockView, updateList(std::filesystem::path("root"), ::testing::Truly([](const std::vector<app::model::BrowserItem>& items) {
        return items.size() == 1U && items[0].displayName == "inside.json";
    })));
    EXPECT_CALL(*mockView, run()).Times(1);

    presenter->start();
}

TEST_F(ListsBrowserPresenterTest, OnDeleteRequestedRoutesByItemType) {
    app::model::BrowserItem dirItem{"dir", "root/dir", true};
    app::model::BrowserItem fileItem{"file.json", "root/file.json", false};

    EXPECT_CALL(*mockDeckManager, deleteFolder(std::filesystem::path("root/dir"))).WillOnce(::testing::Return(true));
    EXPECT_CALL(*mockDeckManager, deleteListFile(std::filesystem::path("root/file.json"))).WillOnce(::testing::Return(true));

    ASSERT_TRUE(mockView->triggerDeleteRequested(dirItem));
    ASSERT_TRUE(mockView->triggerDeleteRequested(fileItem));
}

TEST_F(ListsBrowserPresenterTest, OnNewFolderRequestedCreatesFolderInCurrentDirectory) {
    ASSERT_TRUE(mockView->triggerFolderOpened("root/sub"));

    EXPECT_CALL(*mockDeckManager, createFolder(std::filesystem::path("root/sub/new_folder"))).WillOnce(::testing::Return(true));
    ASSERT_TRUE(mockView->triggerNewFolderRequested("new_folder"));
}

} // namespace app