#include <gtest/gtest.h>
#include "app/model/ListsBrowserViewModel.hpp"

using namespace app::model;
using namespace app;

TEST(ListsBrowserViewModelTest, StripJsonExtension) {
    std::string name1 = "deck.json";
    ListsBrowserViewModel::stripJsonExtension(name1);
    EXPECT_EQ(name1, "deck");

    std::string name2 = "folder";
    ListsBrowserViewModel::stripJsonExtension(name2);
    EXPECT_EQ(name2, "folder");

    std::string name3 = "deck.json.bak";
    ListsBrowserViewModel::stripJsonExtension(name3);
    EXPECT_EQ(name3, "deck.json.bak");
}

TEST(ListsBrowserViewModelTest, UpdateListSelectsFirstItemByDefault) {
    ListsBrowserViewModel vm;
    std::vector<app::model::BrowserItem> items = {
        {"folder1", "path/folder1", true},
        {"deck.json", "path/deck.json", false}
    };

    vm.updateList("path", items);
    EXPECT_EQ(vm.currentPath.string(), "path");
    EXPECT_EQ(vm.selectedIndex, 0);
    EXPECT_TRUE(vm.hasValidSelection());
}

TEST(ListsBrowserViewModelTest, PendingSelectionFromParentDirectory) {
    ListsBrowserViewModel vm;
    vm.currentPath = "path/folder1"; // Simulate currently being inside folder1
    
    std::vector<app::model::BrowserItem> items = {
        {"folder1", "path/folder1", true},
        {"folder2", "path/folder2", true}
    };

    // Navigate "Up" to "path". It should automatically select "folder1"
    vm.updateList("path", items);
    EXPECT_EQ(vm.selectedIndex, 0);
    EXPECT_EQ(vm.pendingSelection, ""); // Should be cleared after use
}

TEST(ListsBrowserViewModelTest, PendingSelectionMatchesWithoutExtension) {
    ListsBrowserViewModel vm;
    vm.pendingSelection = "deck"; // Simulated explicit pending selection (e.g., just created)
    
    std::vector<app::model::BrowserItem> items = {
        {"folder1", "path/folder1", true},
        {"deck.json", "path/deck.json", false}
    };

    vm.updateList("path", items);
    EXPECT_EQ(vm.selectedIndex, 1); // Selects deck.json
}

TEST(ListsBrowserViewModelTest, PrepareRenameStripsExtensionForFiles) {
    ListsBrowserViewModel vm;
    std::vector<app::model::BrowserItem> items = {
        {"deck.json", "path/deck.json", false}
    };
    vm.updateList("path", items);
    vm.prepareRename();
    EXPECT_EQ(vm.inputBuffer, "deck");
}

TEST(ListsBrowserViewModelTest, PrepareRenameKeepsFolderName) {
    ListsBrowserViewModel vm;
    std::vector<app::model::BrowserItem> items = {
        {"my_folder", "path/my_folder", true}
    };
    vm.updateList("path", items);
    vm.prepareRename();
    EXPECT_EQ(vm.inputBuffer, "my_folder");
}