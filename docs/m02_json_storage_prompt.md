# [SYSTEM ROLE]
Act as an Expert C++20 Developer. Your task is to implement the persistence layer (Milestone 2) for our "AI Flashcards" application using Test-Driven Development (TDD).

# [CONTEXT & GOAL]
We have a cleanly separated domain model using `std::shared_ptr` for managing flashcards and decks in memory. You need to implement the `JsonStorage` class (Adapter) that fulfills the `core::IStorage` interface, along with a comprehensive Google Test suite.

# [TECHNICAL REQUIREMENTS]
- **Standard:** C++20.
- **Dependencies:** `nlohmann/json` (for JSON serialization), `<filesystem>`, `<fstream>`, `<memory>`.
- **Testing:** Google Test (GTest).
- **Encoding:** Strict UTF-8 support (must seamlessly handle Korean Hangul and European diacritics).
- **Safety:** All write operations must be atomic to prevent data corruption.

# [ARCHITECTURE & INTERFACES]

## 1. Domain Interface (`include/core/IStorage.hpp` - Already Exists)
```cpp
namespace core {
    namespace fs = std::filesystem;
    class IStorage {
    public:
        virtual ~IStorage() = default;
        virtual std::shared_ptr<FlashcardList> loadList(const fs::path& relativePath) = 0;
        virtual bool saveList(const FlashcardList& list, const fs::path& relativePath) = 0;
        virtual bool deleteList(const fs::path& relativePath) = 0;
        virtual bool moveList(const fs::path& oldPath, const fs::path& newPath) = 0;
        virtual bool createFolder(const fs::path& folderPath) = 0;
        virtual bool deleteFolder(const fs::path& folderPath) = 0;
        virtual bool renameFolder(const fs::path& oldPath, const fs::path& newPath) = 0;
        virtual std::vector<fs::path> getAllAvailableLists() const = 0;
    };
}