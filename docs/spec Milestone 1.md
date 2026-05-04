---
tags:
  - spec
  - architektura
  - mvp
milestone: 1
status: design-complete
---

# High-Level Design Document / Product Requirements Document
### Ustalenia biznesowe i architektoniczne, niezależnie od samego kodu.

## 1. Główne Założenia Projektowe
* **Cel:** Stworzenie wydajnego, wieloplatformowego (Windows/Ubuntu) systemu do nauki słówek metodą Spaced Repetition, z tekstowym interfejsem użytkownika (TUI).
* **Paradygmat Architektoniczny:** Domain-Driven Design (DDD). Bezwzględna separacja Warstwy Domeny (C++ Core) od warstwy prezentacji (UI) i persystencji (JSON/CSV).
* **Zarządzanie Pamięcią:** Zastosowanie `std::shared_ptr` dla obiektów fiszek. Pozwala to na bezpieczne modyfikowanie (edycja, usuwanie, przenoszenie) fiszek w pamięci RAM przez różne komponenty systemu w tym samym czasie.
* **Technologia:** C++20 (logika rdzenia), CMake, Google Test (TDD).

## 2. Funkcjonalności Główne (Core Features)
* **Zarządzanie Listami (Decks):** Tworzenie, usuwanie i pobieranie list fiszek.
* **Zarządzanie Fiszkami (Cards):** Dodawanie, edycja, usuwanie i przenoszenie fiszek między listami.
* **Operacje Masowe (Bulk Operations):** Możliwość grupowego dodawania, usuwania lub przenoszenia zaznaczonych fiszek w celu optymalizacji wydajności.
* **Sesja Nauki (Standard):** Nieskończona pętla powtórek oparta na prawdopodobieństwie (domyślnie: 70% Nowe, 23% Znane, 7% Umiem).
* **Sesja Nauki (Focused):** Skończona sesja nakierowana na przerobienie fiszek o konkretnym statusie (np. tylko "Nowe") od początku do końca.
* **Modyfikacje w Locie:** Możliwość edycji tekstu, cofnięcia ostatniej oceny (Undo), usunięcia fiszki lub przeniesienia jej do innej listy bez przerywania aktywnej sesji nauki.

## 3. Modele i Typy Danych
* **CardState (Stan Pamięciowy):** `New` (Nowa), `Known` (Znana), `Mastered` (Umiem).
* **TranslationDirection (Kierunek Tłumaczenia):** `Front_to_Back`, `Back_to_Front`, `Mixed`. Fiszka przechowuje dwa niezależne stany `CardState` dla obu głównych kierunków.
* **Flashcard (Fiszka):** Podstawowy byt (POCO) posiadający `id`, `text_front`, `text_back` oraz niezależne stany dla kierunków.
* **FlashcardList (Lista Fiszek):** Agregat (Aggregate Root) trzymający kolekcję fiszek w słowniku `std::unordered_map` dla czasu dostępu O(1).
* **DeckManager:** Fasada. Jedyny punkt styku między interfejsem użytkownika a logiką zarządzania listami.
* **ReviewItem:** Struktura używana w trakcie sesji, łącząca wskaźnik na fiszkę (`std::shared_ptr<Flashcard>`) oraz aktualnie testowany kierunek tłumaczenia.

## 4. Algorytm Działania Sesji Nauki
Maszyna stanów (`StudySession`) rozdziela przekazane fiszki do trzech wewnętrznych kolejek (`deque`) na podstawie ich `CardState`.

* **Tryb Standard (Spaced Repetition):**
  1. System losuje kolejkę docelową na podstawie wag zdefiniowanych w `SessionConfig` (np. 70/23/7).
  2. Pobiera pierwszą fiszkę z wylosowanej kolejki. (Jeśli wylosowana kolejka jest pusta, algorytm dynamicznie losuje z pozostałych niepustych kolejek).
  3. Użytkownik ocenia fiszkę (wybiera nowy `CardState`).
  4. Fiszka zmienia swój stan wewnętrzny i ląduje na *końcu* odpowiedniej kolejki w sesji.
* **Tryb Focused (Sekwencyjny):**
  1. Ignoruje wagi i prawdopodobieństwo.
  2. Pobiera kolejno fiszki z jednej zdefiniowanej kolejki (np. tylko `New`).
  3. Po ocenie przez użytkownika, fiszka zmienia stan docelowy, ale *nie wraca* już do puli aktualnej sesji. Sesja kończy się po wyczerpaniu kolejki.

## 5. Przepływ Informacji (Information Flow) i UX
Warstwa UI **nigdy** nie komunikuje się bezpośrednio z klasą `FlashcardList` ani nie modyfikuje wskaźników ręcznie.

1. **Inicjalizacja:** UI prosi `DeckManager` o wczytanie list (lub w M1 - po prostu je tworzy w pamięci).
2. **Start Nauki:** UI pobiera wektor wskaźników do fiszek z danej listy przez `DeckManager` i przekazuje go do nowej instancji `StudySession`.
3. **Pętla Nauki:**
   * UI wywołuje `session->getNextItem()`.
   * UI wyświetla `text_front` (lub `text_back` w zależności od kierunku).
   * Po akcji użytkownika, UI wywołuje `session->submitAnswer()`.
4. **Przerwanie / Modyfikacja:**
   * Jeśli użytkownik zauważy literówkę, UI wywołuje `deckManager->updateCardInList(...)`. Dzięki `std::shared_ptr` zmiana jest natychmiast widoczna w aktywnej sesji.
   * Jeśli użytkownik usunie fiszkę w trakcie nauki, UI wywołuje `deckManager->removeCardFromList(...)`, a następnie powiadamia sesję: `session->removeCardFromSession(...)`.

## 6. User Stories (Historyjki Użytkownika)
* **US1:** Jako Użytkownik, chcę rozpocząć nieskończoną sesję nauki, w której system częściej (70%) pyta mnie o słówka nowe, a rzadziej (7%) o te, które już umiem.
* **US2:** Jako Użytkownik, chcę móc odwrócić kierunek nauki (Język 2 -> Język 1), aby ćwiczyć rozpoznawanie ze słuchu/czytania, a system musi pamiętać mój postęp niezależnie dla tego kierunku. (oddzielne CardState dla Front_to_Back i Back_to_Front)
* **US3:** Jako Użytkownik, widząc błąd ortograficzny w wyświetlanej fiszce, chcę ją natychmiast wyedytować bez wychodzenia z trybu nauki, a kolejna runda powtórek ma już pokazać poprawioną wersję.
* **US4:** Jako Użytkownik, chcę zaznaczyć 20 trudnych fiszek na liście i za pomocą jednego kliknięcia przenieść je do osobnej listy "Trudne słówka" (Bulk Operation), aby móc odpalić na nich sesję trybu Focused.
* **US5:** Jako Użytkownik, chcę móc cofnąć swoją ostatnią ocenę w trakcie sesji (Undo), jeśli przez pomyłkę kliknąłem "Umiem" zamiast "Nowe", przywracając fiszkę do odpowiedniej kolejki.
