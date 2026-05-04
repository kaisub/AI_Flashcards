# High-Level Design: Milestone 3 – Interfejs Użytkownika (FTXUI)

## 1. Główne Założenia Projektowe

- **Cel:** Stworzenie responsywnego, tekstowego interfejsu użytkownika (TUI), który umożliwia zarządzanie bazą wiedzy oraz przeprowadzanie sesji nauki.
    
- **Wzorzec Projektowy:** **Model-View-Controller (MVC) / State Machine**.
    
    - `Model` = `DeckManager` i `StudySession` (Milestone 1) + `IStorage` (Milestone 2).
        
    - `View` = Komponenty FTXUI.
        
    - `Controller` = `AppController` (główna pętla i zarządca stanu aplikacji).
        
- **Technologia:** C++20, biblioteka **FTXUI** (FetchContent w CMake).
    
- **Separacja (Zasada DIP):** Widoki nie znają struktury pamięciowej plików ani wewnętrznych wskaźników do bazy poza tymi, które dostarczy im `AppController`. Wymiana FTXUI na Qt w przyszłości ma wymagać jedynie przepięcia warstwy View.
    

## 2. Architektura Aplikacji (App Layer)

Aplikacja będzie oparta na maszynie stanów zdefiniowanej w głównej klasie `AppController`. Klasa ta inicjalizuje silnik FTXUI (`ftxui::ScreenInteractive`) oraz zarządza przełączaniem się między głównymi ekranami.

### Główne Stany Aplikacji (App States):

1. **`State::FileBrowser` (Eksplorator Bazy):** - Pokazuje zawartość obecnego katalogu. Zawsze zaczyna od `_basePath`.
    
    - Zwraca listę folderów i plików `.json`.
        
    - Pozwala wejść głębiej (przeładowuje widok) lub cofnąć się wyżej.
        
2. **`State::DeckEditor` (Edytor Listy):**
    
    - Włącza się po wejściu w plik `.json`.
        
    - Wyświetla tabelę/listę fiszek należących do danej grupy.
        
    - Umożliwia dodawanie, usuwanie i edycję (triggeruje natychmiastowy zapis w `IStorage`).
        
3. **`State::StudySession` (Tryb Nauki):**
    
    - Aktywuje pętlę pytań. Czerpie dane z obiektu `StudySession`.
        
    - Zarządza wyświetlaniem frontu, tyłu (odwrócenie) i ocenianiem (`New`, `Known`, `Mastered`).
        
4. **`State::GlobalSettings` (Ustawienia):**
    
    - Konfiguracja wag losowania dla trybu Standard, domyślnego kierunku tłumaczenia itp.
        

## 3. Strategia Synchronizacji i Bezpieczeństwa (Zgodnie z M2)

Aby zapewnić wydajność i jednocześnie integralność wiedzy, `AppController` będzie wymuszał na `DeckManager` następujące akcje:

- **Modyfikacja strukturalna (CRUD na fiszkach/listach):** Każde wciśnięcie "Zapisz" po edycji tekstu w locie (US3) natychmiast wywołuje asynchroniczny `SaveList()`. Zapobiega to utracie wprowadzonych zmian.
    
- **Postęp nauki (`CardState`):** W trakcie intensywnego "klikania" ocen (Umiem/Nie Umiem), aktualizowany jest wyłącznie stan w pamięci RAM (`std::shared_ptr<Flashcard>`). Dopiero wyjście ze `State::StudySession` do `State::DeckEditor` wykonuje ostateczny "zrzut" (Commit) do pliku JSON na dysku.
    

## 4. Przepływ Zdarzeń (Event Flow) na przykładzie Trybu Nauki

Jak dane płyną przez warstwy bez łamania enkapsulacji:

1. Użytkownik znajduje się w `FileBrowser`, wybiera listę "Angielski_IT.json" i klika `[Rozpocznij Naukę]`.
    
2. UI (widok) emituje zdarzenie do `AppController`: `Command::StartSession("Angielski_IT")`.
    
3. `AppController` pobiera wskaźniki na fiszki z `DeckManager` i tworzy nową instancję `StudySession`.
    
4. `AppController` przełącza stan interfejsu na `StudySessionView` i podaje mu obiekt (lub strukturę DTO) pierwszej fiszki: `session->getNextItem()`.
    
5. Użytkownik ocenia fiszkę klikając przycisk/skrót klawiszowy (np. `K` - Known).
    
6. Widok informuje kontroler: `Command::RateCard(CardState::Known)`.
    
7. `AppController` przekazuje to do logiki: `session->submitAnswer()`. Następnie natychmiast pyta o kolejną fiszkę i odświeża widok.
    

## 5. User Stories dla TUI (Milestone 3)

- **US1 (Nawigacja):** Jako użytkownik, chcę widzieć moje listy podzielone na kategorie (foldery) w formie menu, w które mogę wchodzić i z których mogę wychodzić, aby łatwo odnaleźć interesujący mnie temat.
    
- **US2 (Responsywność):** Jako użytkownik terminala, chcę móc sterować aplikacją zarówno za pomocą myszki (klikanie w przyciski), jak i skrótów klawiszowych (strzałki, Enter, Esc), aby moja nauka była błyskawiczna.
    
- **US3 (Integracja Edycji):** Jako użytkownik uczący się, chcę w dowolnym momencie wcisnąć przycisk "Edytuj" pod wyświetlaną fiszką, poprawić w niej literówkę we wbudowanym polu tekstowym i natychmiast kontynuować sesję, wiedząc, że zmiana trwale zapisała się na dysku.
