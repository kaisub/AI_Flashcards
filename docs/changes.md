Milestone 3
===========

I've successfully implemented the Pre-Study Configuration screen. Here's a summary of all the changes:

New Files Created:

 1 include/app/views/IStudyConfigView.hpp - Interface for the configuration view
 2 include/app/views/FtxuiStudyConfigView.hpp - Header for FTXUI implementation
 3 src/app/views/FtxuiStudyConfigView.cpp - FTXUI implementation with Polish UI labels

Modified Files:

 1 include/app/AppState.hpp - Added StudyConfig state
 2 include/app/AppController.hpp - Added config view support and new methods
 3 src/app/AppController.cpp - Implemented config handling logic
 4 src/main.cpp - Instantiated and injected the config view
 5 CMakeLists.txt - Added the new cpp file to build

Key Features Implemented:

 • Pre-study configuration screen with three sections:
    • Kierunek (Direction): Przód->Tył, Tył->Przód, Mieszany
    • Tryb (Mode): Standardowy, Skupiony (Focused)
    • Cel (Target State): Nowe, Znane, Opanowane (grayed out when not in Focused mode)
 • Start button that creates the StudySession with configured parameters
 • Cancel button that returns to DeckEditor
 • The target state selection is visually disabled when Standard mode is selected
 • Proper state transitions: DeckEditor → StudyConfig → StudySession

Milestone 1
===========

Explanation of Changes and Design Decisions
-------------------------------------------

1. Header renaming (.h -> .hpp)
     - Resolved naming inconsistency by using .hpp consistently for core domain headers.
     - Affected headers: Flashcard, FlashcardList, DeckManager, StudySession.
     - Updated CMakeLists and include paths accordingly.

2. Flashcard.hpp
     - Added operator== and operator!= for easier test assertions.
     - Helps with shared_ptr identity checks and ReviewItem comparisons.

3. FlashcardList.cpp
     - addCard, removeCard, updateCard, getCard:
         Implemented standard CRUD behavior using std::unordered_map for average O(1) operations.
     - importCardsFrom overloads:
         Accept shared_ptr<Flashcard> directly or import from another FlashcardList.
         Existing shared_ptr instances are preserved to maintain shared ownership.
         Result: state updates in one list are visible in other lists holding the same card pointer.

4. DeckManager.cpp
     - Acts as a facade over FlashcardList instances.
     - createList, deleteList, getList, getAllListNames:
         Manage the collection of FlashcardList objects.
     - addCardToList, removeCardFromList, updateCardInList:
         Delegate behavior to FlashcardList.
     - moveCard and moveCards:
         A move first tries addCard to the target list, then removes from the source only if add succeeds.
         This protects against data loss when target already contains the same card ID and preserves shared ownership semantics.

5. StudySession.cpp
     - Constructor and initializeQueues:
         Builds queueNew, queueKnown, queueMastered from initial CardState and TranslationDirection.
         Uses std::shuffle to randomize queue order.
     - getNextItem:
         Delegates to drawStandard() or drawFocused() based on currentConfig.type.
     - drawStandard:
         Builds weightedQueues from non-empty queues only.
         This naturally handles weight normalization when any queue is empty.
         Uses std::discrete_distribution with currentConfig weights.
         Pops the next ReviewItem and records a HistoryRecord for undo support.
     - drawFocused:
         Draws from the queue selected by currentConfig.focusedTargetState.
     - submitAnswer:
         Updates state_Front_to_Back or state_Back_to_Front based on askedDirection.
         Stores pushedToQueueState in the latest HistoryRecord.
         Re-queues the card into New, Known, or Mastered deque according to new state.
         When direction is Mixed, re-queued items may use a newly randomized direction.
     - undoLastAction:
         Pops the last HistoryRecord.
         Removes the card from whichever queue it was re-queued into.
         Restores previous front/back states.
         Re-adds the card to the queue matching the restored state.
     - removeCardFromSession and removeCardFromAnyQueue:
         Fully removes a specific card from the active session by scanning all queues.

6. StudySession_test.cpp
     - StandardMode_RespectsDrawProbabilities:
         Uses 5000 iterations and EXPECT_NEAR with 10% tolerance for weighted draw behavior.
     - StandardMode_HandlesEmptyQueues:
         Verifies continued drawing when one queue is exhausted.
     - FocusedMode_FiniteExhaustion:
         Confirms focused draws stop when the targeted queue is empty.
     - SubmitAnswer_UpdatesStateAndQueue:
         Checks state transitions and queue movement after answer submission.
     - UndoAction_RestoresPreviousState:
         Verifies full state and queue rollback across multiple undos.
     - RemoveCard_SafelyEjectsCard:
         Ensures removed cards are no longer drawable.
     - MixedDirectionHandling:
         Confirms state updates affect only askedDirection and undo restores it correctly.

7. CMakeLists.txt
     - Added src/FlashcardList.cpp, src/DeckManager.cpp, src/StudySession.cpp to core_lib.
    - Updated target_include_directories to include include/core.
     - Added tests/FlashcardList_test.cpp, tests/DeckManager_test.cpp, tests/StudySession_test.cpp to flashcards_tests.
