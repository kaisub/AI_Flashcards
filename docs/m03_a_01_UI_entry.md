Act as a Senior C++ Developer. We are implementing Milestone 3A (Application Layer) for the AI Flashcards project. 
The goal is to implement the AppController and View Interfaces without any TUI/GUI library yet.

Please proceed STEP BY STEP.
Do not commit, I'll do that after approving.

Architecture Context for Aider:
This project follows Domain-Driven Design. The AppController is part of the Application Layer. It must coordinate between the core::DeckManager (logic) and core::IStorage (persistence).
Key rule: UI Views are abstract interfaces. The Controller must NOT know about FTXUI or any specific rendering logic.
Data Consistency: Content changes (text) must be saved immediately via IStorage::saveList(), but learning progress (CardState) is committed only when exiting the StudySession or DeckEditor.

STEP 1 of 3:
Create the directory structure: `include/app/`, `include/app/views/`, and `src/app/`.
Create the following header files based on our design:
- `include/app/AppState.hpp` (Enum: ListsBrowser, DeckEditor, StudySession, Settings, Exit)
- `include/app/views/IView.hpp` (Base interface with virtual void run() = 0)
- `include/app/views/IListsBrowserView.hpp` (Lists browser interface)
- `include/app/AppController.hpp` (Controller class definition)

All comments in the code must be in ENGLISH. 
Use modern C++20 features. 
Ensure proper includes and namespace `app`.
