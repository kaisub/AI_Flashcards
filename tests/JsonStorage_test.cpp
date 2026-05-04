#include "gtest/gtest.h"
#include "storage/JsonStorage.hpp"
#include "core/FlashcardList.hpp"
#include "core/Flashcard.hpp"
#include "utils/TestFactory.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Helper to create a unique temporary directory for each test fixture
class JsonStorageTest : public ::testing::Test {
protected:
    fs::path temp_dir;
    std::unique_ptr<storage::JsonStorage> storage;

    void SetUp() override {
        // Create a unique temporary directory for each test
        temp_dir = fs::temp_directory_path() / fs::path("json_storage_test_" +
                                            std::to_string(std::hash<std::string>{}(
                                                ::testing::UnitTest::GetInstance()->current_test_info()->name())) +
                                            std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        fs::create_directories(temp_dir);
        storage = std::make_unique<storage::JsonStorage>(temp_dir);
    }

    void TearDown() override {
        // Clean up the temporary directory
        if (fs::exists(temp_dir)) {
            fs::remove_all(temp_dir);
        }
    }

    // Helper to read file content for verification (e.g., UTF-8)
    std::string readFileContent(const fs::path& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filePath.string());
        }
        std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
        return content;
    }
};

// --- Test JsonStorage Constructor ---
TEST_F(JsonStorageTest, ConstructorCreatesBasePathIfNotExists) {
    fs::path nonExistentPath = temp_dir / "new_base";
    ASSERT_FALSE(fs::exists(nonExistentPath));
    storage::JsonStorage newStorage(nonExistentPath);
    EXPECT_TRUE(fs::exists(nonExistentPath));
    EXPECT_TRUE(fs::is_directory(nonExistentPath));
    fs::remove_all(nonExistentPath); // Clean up
}

// --- Test loadList with invalid JSON syntax ---
TEST_F(JsonStorageTest, LoadInvalidJsonSyntaxThrowsError) {
    fs::path listPath = "invalid_syntax.json";
    fs::path fullPath = temp_dir / listPath;

    // Test 1: Missing closing braces/brackets, JSON syntax error
    std::ofstream ofs1(fullPath);
    ofs1 << "{ \"name\": \"MalformedDeck\", \"cards\": [ { \"id\": \"1\", \"text_front\": \"Front\" }"; // Missing closing brace for card object and cards array, and main object
    ofs1.close();
    
    try {
        storage->loadList(listPath);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("JSON parsing error"), std::string::npos);
    }

    // Test 2: Completely invalid JSON
    std::ofstream ofs2(fullPath);
    ofs2 << "This is not JSON at all.";
    ofs2.close();
    EXPECT_THROW(storage->loadList(listPath), std::runtime_error);
}

// --- Test loadList wraps mapper exceptions ---
TEST_F(JsonStorageTest, LoadListWrapsSchemaExceptions) {
    fs::path listPath = "schema_error.json";
    fs::path fullPath = temp_dir / listPath;
    
    // Valid JSON syntax, but missing mandatory fields ("name" and "cards")
    std::ofstream ofs(fullPath);
    ofs << "{ \"some_other_field\": \"value\" }"; 
    ofs.close();

    try {
        storage->loadList(listPath);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        // Verify that JsonStorage caught the mapper error and wrapped it in its own error message
        EXPECT_NE(std::string(e.what()).find("Data format error"), std::string::npos);
    }
}

TEST_F(JsonStorageTest, ConstructorThrowsIfBasePathIsNotDirectory) {
    fs::path filePath = temp_dir / "not_a_dir.txt";
    std::ofstream ofs(filePath);
    if (!ofs.is_open()) {
        FAIL() << "Could not create file for test: " << filePath;
    }
    ofs << "some content"; // Create a file
    ofs.close();
    ASSERT_TRUE(fs::exists(filePath));
    ASSERT_TRUE(fs::is_regular_file(filePath));

    EXPECT_THROW(storage::JsonStorage newStorage(filePath), std::runtime_error);
}

// --- Test saveList and loadList ---
TEST_F(JsonStorageTest, SaveAndLoadBasicList) {
    core::FlashcardList list("MyDeck");
    list.addCard(test_utils::createTestFlashcard("1", "Hello", "World"));
    list.addCard(test_utils::createTestFlashcard("2", "Test", "Karte"));

    fs::path listPath = "my_deck.json";
    EXPECT_TRUE(storage->saveList(list, listPath));

    auto loadedList = storage->loadList(listPath);
    ASSERT_NE(loadedList, nullptr);
    EXPECT_EQ(loadedList->getName(), "MyDeck");
    EXPECT_EQ(loadedList->size(), 2u);
    EXPECT_NE(loadedList->getCard("1"), nullptr);
    EXPECT_NE(loadedList->getCard("2"), nullptr);
    EXPECT_EQ(loadedList->getCard("1")->text_front, "Hello");
    EXPECT_EQ(loadedList->getCard("2")->text_back, "Karte");
}

TEST_F(JsonStorageTest, SaveAndLoadListWithUTF8) {
    core::FlashcardList list("UTF8Deck");
    list.addCard(test_utils::createTestFlashcard("1", "Cześć", "Świat")); // Polish
    list.addCard(test_utils::createTestFlashcard("2", "안녕하세요", "세상")); // Korean
    list.addCard(test_utils::createTestFlashcard("3", "Bonjour", "Monde")); // French
    list.addCard(test_utils::createTestFlashcard("4", "你好", "世界")); // Chinese

    fs::path listPath = "utf8_deck.json";
    EXPECT_TRUE(storage->saveList(list, listPath));

    // Verify file content directly for UTF-8 (optional, loadList should verify)
    std::string content = readFileContent(temp_dir / listPath);
    EXPECT_TRUE(content.find("Cześć") != std::string::npos);
    EXPECT_TRUE(content.find("안녕하세요") != std::string::npos);
    EXPECT_TRUE(content.find("Bonjour") != std::string::npos);
    EXPECT_TRUE(content.find("你好") != std::string::npos);

    auto loadedList = storage->loadList(listPath);
    ASSERT_NE(loadedList, nullptr);
    EXPECT_EQ(loadedList->getName(), "UTF8Deck");
    EXPECT_EQ(loadedList->size(), 4u);
    EXPECT_EQ(loadedList->getCard("1")->text_front, "Cześć");
    EXPECT_EQ(loadedList->getCard("2")->text_back, "세상");
    EXPECT_EQ(loadedList->getCard("3")->text_front, "Bonjour");
    EXPECT_EQ(loadedList->getCard("4")->text_back, "世界");
}

TEST_F(JsonStorageTest, LoadNonExistentListReturnsNullptr) {
    fs::path listPath = "non_existent.json";
    EXPECT_EQ(storage->loadList(listPath), nullptr);
}

TEST_F(JsonStorageTest, SaveListAtomicOperation) {
    core::FlashcardList originalList("Original");
    originalList.addCard(test_utils::createTestFlashcard("A", "Original Front", "Original Back"));

    fs::path listPath = "atomic_test.json";
    fs::path fullPath = temp_dir / listPath;
    fs::path tempFilePath = temp_dir / (listPath.string() + ".tmp");

    // Save the original list first
    ASSERT_TRUE(storage->saveList(originalList, listPath));
    ASSERT_TRUE(fs::exists(fullPath));
    ASSERT_FALSE(fs::exists(tempFilePath)); // Temp file should be gone

    // Simulate a failure during save (e.g., permissions error on rename, or disk full)
    // This is hard to truly test directly as std::filesystem::rename is OS-atomic.
    // The main point is that if saveList throws, the original file is not corrupted.
    core::FlashcardList corruptList("Corrupt");
    corruptList.addCard(test_utils::createTestFlashcard("Z", "Corrupt Front", "Corrupt Back"));

    // To simulate failure *before* atomic rename, we can try to save to a read-only directory
    // or provide an invalid path. But for now, we rely on the implementation of `std::filesystem::rename`
    // to either succeed atomically or leave the original untouched.
    // We can at least test that if an error *does* occur during the write to temp file,
    // the original file remains intact and the temp file is cleaned up.

    // Force an error during saving (e.g., trying to save to an invalid character path, or full disk simulation)
    // For this test, we can only verify the *expected* behavior if saveList *succeeds* atomically,
    // or if a previous file remains intact if an error occurs *before* the rename.

    // Let's modify the list and save, then verify it was saved correctly and atomically.
    core::FlashcardList newList("Updated");
    newList.addCard(test_utils::createTestFlashcard("B", "Updated Front", "Updated Back"));
    EXPECT_TRUE(storage->saveList(newList, listPath));

    auto loadedList = storage->loadList(listPath);
    ASSERT_NE(loadedList, nullptr);
    EXPECT_EQ(loadedList->getName(), "Updated");
    EXPECT_EQ(loadedList->size(), 1u);
    EXPECT_EQ(loadedList->getCard("B")->text_front, "Updated Front");
    EXPECT_FALSE(fs::exists(tempFilePath)); // Temp file should still be gone
}


TEST_F(JsonStorageTest, SaveAndLoadListWithCardStates) {
    core::FlashcardList list("StateDeck");
    list.addCard(test_utils::createTestFlashcard("1", "New", "Card", core::CardState::New, core::CardState::New));
    list.addCard(test_utils::createTestFlashcard("2", "Known", "Card", core::CardState::Known, core::CardState::Known));
    list.addCard(test_utils::createTestFlashcard("3", "Mastered", "Card", core::CardState::Mastered, core::CardState::Mastered));

    fs::path listPath = "state_deck.json";
    EXPECT_TRUE(storage->saveList(list, listPath));

    auto loadedList = storage->loadList(listPath);
    ASSERT_NE(loadedList, nullptr);
    EXPECT_EQ(loadedList->getName(), "StateDeck");
    EXPECT_EQ(loadedList->size(), 3u);

    ASSERT_NE(loadedList->getCard("1"), nullptr);
    EXPECT_EQ(loadedList->getCard("1")->state_Front_to_Back, core::CardState::New);
    EXPECT_EQ(loadedList->getCard("1")->state_Back_to_Front, core::CardState::New);

    ASSERT_NE(loadedList->getCard("2"), nullptr);
    EXPECT_EQ(loadedList->getCard("2")->state_Front_to_Back, core::CardState::Known);
    EXPECT_EQ(loadedList->getCard("2")->state_Back_to_Front, core::CardState::Known);

    ASSERT_NE(loadedList->getCard("3"), nullptr);
    EXPECT_EQ(loadedList->getCard("3")->state_Front_to_Back, core::CardState::Mastered);
    EXPECT_EQ(loadedList->getCard("3")->state_Back_to_Front, core::CardState::Mastered);
}

// --- Test deleteList ---
TEST_F(JsonStorageTest, DeleteExistingList) {
    core::FlashcardList list("ToDelete");
    list.addCard(test_utils::createTestFlashcard("1", "A", "B"));
    fs::path listPath = "to_delete.json";
    storage->saveList(list, listPath);
    ASSERT_TRUE(fs::exists(temp_dir / listPath));

    EXPECT_TRUE(storage->deleteList(listPath));
    EXPECT_FALSE(fs::exists(temp_dir / listPath));
}

TEST_F(JsonStorageTest, DeleteNonExistentListReturnsFalse) {
    fs::path listPath = "non_existent_to_delete.json";
    ASSERT_FALSE(fs::exists(temp_dir / listPath));
    EXPECT_FALSE(storage->deleteList(listPath));
}

// --- Test moveList ---
TEST_F(JsonStorageTest, MoveExistingList) {
    core::FlashcardList list("ToMove");
    list.addCard(test_utils::createTestFlashcard("1", "A", "B"));
    fs::path oldPath = "old_path.json";
    fs::path newPath = "new_path.json";
    storage->saveList(list, oldPath);
    ASSERT_TRUE(fs::exists(temp_dir / oldPath));
    ASSERT_FALSE(fs::exists(temp_dir / newPath));

    EXPECT_TRUE(storage->moveList(oldPath, newPath));
    EXPECT_FALSE(fs::exists(temp_dir / oldPath));
    EXPECT_TRUE(fs::exists(temp_dir / newPath));

    auto loadedList = storage->loadList(newPath);
    ASSERT_NE(loadedList, nullptr);
    EXPECT_EQ(loadedList->getName(), "ToMove");
}

TEST_F(JsonStorageTest, MoveListToSubfolder) {
    core::FlashcardList list("ToMoveSub");
    list.addCard(test_utils::createTestFlashcard("1", "X", "Y"));
    fs::path oldPath = "list_to_move.json";
    fs::path newFolderPath = "subfolder";
    fs::path newPath = newFolderPath / "moved_list.json";

    storage->saveList(list, oldPath);
    ASSERT_TRUE(fs::exists(temp_dir / oldPath));
    ASSERT_FALSE(fs::exists(temp_dir / newFolderPath));

    EXPECT_TRUE(storage->moveList(oldPath, newPath));
    EXPECT_FALSE(fs::exists(temp_dir / oldPath));
    EXPECT_TRUE(fs::exists(temp_dir / newPath));
    EXPECT_TRUE(fs::is_regular_file(temp_dir / newPath));
    EXPECT_TRUE(fs::is_directory(temp_dir / newFolderPath)); // Subfolder should be created

    auto loadedList = storage->loadList(newPath);
    ASSERT_NE(loadedList, nullptr);
    EXPECT_EQ(loadedList->getName(), "ToMoveSub");
}

TEST_F(JsonStorageTest, MoveNonExistentListReturnsFalse) {
    fs::path oldPath = "non_existent_old.json";
    fs::path newPath = "new_path.json";
    EXPECT_FALSE(storage->moveList(oldPath, newPath));
}

TEST_F(JsonStorageTest, MoveListToExistingTargetReturnsFalse) {
    core::FlashcardList list1("List1");
    storage->saveList(list1, "list1.json");
    core::FlashcardList list2("List2");
    storage->saveList(list2, "list2.json");

    EXPECT_FALSE(storage->moveList("list1.json", "list2.json"));
}

// --- Test createFolder ---
TEST_F(JsonStorageTest, CreateNewFolder) {
    fs::path folderPath = "new_folder";
    EXPECT_TRUE(storage->createFolder(folderPath));
    EXPECT_TRUE(fs::exists(temp_dir / folderPath));
    EXPECT_TRUE(fs::is_directory(temp_dir / folderPath));
}

TEST_F(JsonStorageTest, CreateNestedFolders) {
    fs::path folderPath = "parent/child/grandchild";
    EXPECT_TRUE(storage->createFolder(folderPath));
    EXPECT_TRUE(fs::exists(temp_dir / folderPath));
    EXPECT_TRUE(fs::is_directory(temp_dir / folderPath));
}

TEST_F(JsonStorageTest, CreateExistingFolderReturnsTrue) {
    fs::path folderPath = "existing_folder";
    fs::create_directory(temp_dir / folderPath);
    EXPECT_TRUE(storage->createFolder(folderPath)); // Should return true as it effectively exists
    EXPECT_TRUE(fs::exists(temp_dir / folderPath));
}

// --- Test deleteFolder ---
TEST_F(JsonStorageTest, DeleteEmptyFolder) {
    fs::path folderPath = "empty_folder";
    fs::create_directory(temp_dir / folderPath);
    ASSERT_TRUE(fs::exists(temp_dir / folderPath));

    EXPECT_TRUE(storage->deleteFolder(folderPath));
    EXPECT_FALSE(fs::exists(temp_dir / folderPath));
}

TEST_F(JsonStorageTest, DeleteNonExistentFolderReturnsFalse) {
    fs::path folderPath = "non_existent_folder";
    ASSERT_FALSE(fs::exists(temp_dir / folderPath));
    EXPECT_FALSE(storage->deleteFolder(folderPath));
}

TEST_F(JsonStorageTest, DeleteNonEmptyFolder) {
    fs::path folderPath = "non_empty_folder";
    fs::create_directory(temp_dir / folderPath);
    std::ofstream(temp_dir / folderPath / "file.txt") << "content"; // Add a file
    ASSERT_TRUE(fs::exists(temp_dir / folderPath / "file.txt"));

    EXPECT_TRUE(storage->deleteFolder(folderPath)); // Should succeed due to remove_all
    EXPECT_FALSE(fs::exists(temp_dir / folderPath)); // Folder should be gone
}

// --- Test renameFolder ---
TEST_F(JsonStorageTest, RenameExistingFolder) {
    fs::path oldPath = "old_folder";
    fs::path newPath = "new_folder";
    fs::create_directory(temp_dir / oldPath);
    std::ofstream(temp_dir / oldPath / "file.txt") << "content"; // Add content

    EXPECT_TRUE(storage->renameFolder(oldPath, newPath));
    EXPECT_FALSE(fs::exists(temp_dir / oldPath));
    EXPECT_TRUE(fs::exists(temp_dir / newPath));
    EXPECT_TRUE(fs::is_directory(temp_dir / newPath));
    EXPECT_TRUE(fs::exists(temp_dir / newPath / "file.txt")); // Content should be moved
}

TEST_F(JsonStorageTest, RenameNonExistentFolderReturnsFalse) {
    fs::path oldPath = "non_existent_old_folder";
    fs::path newPath = "new_folder";
    EXPECT_FALSE(storage->renameFolder(oldPath, newPath));
}

TEST_F(JsonStorageTest, RenameFolderToExistingTargetReturnsFalse) {
    fs::path folder1 = "folder1";
    fs::path folder2 = "folder2";
    fs::create_directory(temp_dir / folder1);
    fs::create_directory(temp_dir / folder2);

    EXPECT_FALSE(storage->renameFolder(folder1, folder2));
}

TEST_F(JsonStorageTest, RenameFolderToNestedPath) {
    fs::path oldPath = "outer_folder";
    fs::path newPath = "parent/new_nested_folder";
    fs::create_directory(temp_dir / oldPath);
    std::ofstream(temp_dir / oldPath / "test_file.txt") << "hello";

    EXPECT_TRUE(storage->renameFolder(oldPath, newPath));
    EXPECT_FALSE(fs::exists(temp_dir / oldPath));
    EXPECT_TRUE(fs::exists(temp_dir / newPath));
    EXPECT_TRUE(fs::is_directory(temp_dir / "parent")); // Parent should be created
    EXPECT_TRUE(fs::exists(temp_dir / newPath / "test_file.txt"));
}

// --- Test getAllAvailableLists ---
TEST_F(JsonStorageTest, GetAllAvailableListsFromEmptyBase) {
    auto lists = storage->getAllAvailableLists();
    EXPECT_TRUE(lists.empty());
}

TEST_F(JsonStorageTest, GetAllAvailableListsFlat) {
    storage->saveList(core::FlashcardList("List1"), "list1.json");
    storage->saveList(core::FlashcardList("List2"), "list2.json");
    std::ofstream(temp_dir / "other.txt") << "not a json"; // Not a JSON file

    auto lists = storage->getAllAvailableLists();
    EXPECT_EQ(lists.size(), 2u);
    std::vector<fs::path> expected = {"list1.json", "list2.json"};
    std::sort(lists.begin(), lists.end()); // Sort for consistent comparison
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(lists, expected);
}

TEST_F(JsonStorageTest, GetAllAvailableListsNested) {
    storage->createFolder("subfolder1");
    storage->createFolder("subfolder2/nested");

    storage->saveList(core::FlashcardList("ListA"), "listA.json");
    storage->saveList(core::FlashcardList("ListB"), "subfolder1/listB.json");
    storage->saveList(core::FlashcardList("ListC"), "subfolder2/nested/listC.json");
    std::ofstream(temp_dir / "subfolder1" / "notjson.txt") << "content";

    auto lists = storage->getAllAvailableLists();
    EXPECT_EQ(lists.size(), 6u);
    std::vector<fs::path> expected = {
        "listA.json", 
        "subfolder1", 
        "subfolder1/listB.json", 
        "subfolder2", 
        "subfolder2/nested", 
        "subfolder2/nested/listC.json"
    };
    std::sort(lists.begin(), lists.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(lists, expected);
}

TEST_F(JsonStorageTest, GetAllAvailableListsFromNonExistentBase) {
    fs::remove_all(temp_dir); // Remove the base path
    ASSERT_FALSE(fs::exists(temp_dir));

    storage::JsonStorage nonExistentStorage(temp_dir); // Should create it but it's empty
    auto lists = nonExistentStorage.getAllAvailableLists();
    EXPECT_TRUE(lists.empty());
}

// --- Additional edge case tests ---

TEST_F(JsonStorageTest, LoadListWhenPathIsDirectoryReturnsNullptr) {
    fs::path dirPath = "some_dir.json"; // unusual but valid name
    fs::create_directory(temp_dir / dirPath);
    ASSERT_TRUE(fs::is_directory(temp_dir / dirPath));
    EXPECT_EQ(storage->loadList(dirPath), nullptr);
}

TEST_F(JsonStorageTest, SaveAndLoadEmptyList) {
    core::FlashcardList list("EmptyDeck");
    fs::path listPath = "empty_deck.json";
    EXPECT_TRUE(storage->saveList(list, listPath));

    auto loadedList = storage->loadList(listPath);
    ASSERT_NE(loadedList, nullptr);
    EXPECT_EQ(loadedList->getName(), "EmptyDeck");
    EXPECT_EQ(loadedList->size(), 0u);
}

TEST_F(JsonStorageTest, SaveListCreatesNestedParentDirectories) {
    core::FlashcardList list("NestedDeck");
    list.addCard(test_utils::createTestFlashcard("1", "A", "B"));
    fs::path listPath = "auto_created/nested/deck.json";
    ASSERT_FALSE(fs::exists(temp_dir / "auto_created"));

    EXPECT_TRUE(storage->saveList(list, listPath));
    EXPECT_TRUE(fs::exists(temp_dir / listPath));

    auto loadedList = storage->loadList(listPath);
    ASSERT_NE(loadedList, nullptr);
    EXPECT_EQ(loadedList->getName(), "NestedDeck");
}

TEST_F(JsonStorageTest, DeleteListOnDirectoryReturnsFalse) {
    fs::path dirPath = "some_folder";
    fs::create_directory(temp_dir / dirPath);
    ASSERT_TRUE(fs::is_directory(temp_dir / dirPath));
    // deleteList should not delete directories
    EXPECT_FALSE(storage->deleteList(dirPath));
    EXPECT_TRUE(fs::exists(temp_dir / dirPath));
}
