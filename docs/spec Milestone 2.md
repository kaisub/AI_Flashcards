---
tags: [specyfikacja, architektura, cpp, json, persistence]
milestone: 2
status: design-complete
---

# High-Level Design: Milestone 2 – Warstwa Persystencji (JSON Storage)

## 1. Główne Założenia Projektowe
* **Cel:** Zapewnienie trwałego, bezpiecznego i wieloplatformowego (Windows/Ubuntu) składowania danych użytkownika (list fiszek) przy zachowaniu pełnej separacji od logiki domenowej.
* **Wzorzec Projektowy:** **Adapter / Repository**. Implementacja konkretna (`JsonStorage`) ukryta za interfejsem wirtualnym (`IStorage`), co umożliwia łatwą wymianę mechanizmu składowania w przyszłości.
* **Zasada Dependency Inversion (DIP):** Moduł `Core` definiuje interfejs, a moduł `Infrastructure` dostarcza implementację.
* **Technologia:** C++20, `std::filesystem` dla operacji na ścieżkach, `nlohmann/json` jako silnik serializacji.
* **Bezpieczeństwo danych:** Priorytetem jest integralność plików (Atomic Writes) oraz wsparcie dla kodowania **UTF-8** (obsługa wielu języków, w tym koreańskiego).

## 2. Funkcjonalności Główne (Core Features)
* **Zarządzanie Hierarchią:** Tworzenie, usuwanie i zmiana nazw folderów na fizycznym dysku.
* **Operacje na Listach:** Zapis (`saveList`), odczyt (`loadList`), usuwanie oraz przenoszenie plików `.json` między folderami.
* **Atomic Write (Bezpieczny Zapis):** Mechanizm zapisu do pliku tymczasowego `.tmp` i zamiany (rename), zapobiegający uszkodzeniu bazy danych przy awarii zasilania/procesu.
* **Synchronizacja Selektywna:** Natychmiastowy zapis przy edycji treści (tekst fiszki) oraz zapis zbiorczy stanu nauki (`CardState`) przy wyjściu z modułu sesji.
* **Discovery:** Automatyczne skanowanie katalogu głównego w celu wykrycia wszystkich dostępnych list fiszek w strukturze drzewiastej.

## 3. Modele i Typy Danych
* **IStorage (Interface):** Czysto wirtualna klasa bazowa definiująca kontrakt operacji na systemie plików. Operuje na `std::filesystem::path`.
* **JsonStorage (Adapter):** Konkretna implementacja infrastrukturalna zarządzająca fizyczną ścieżką `rootDir`.
* **Mapowanie JSON:**
    * `FlashcardList` -> Obiekt główny JSON (nazwa listy, tablica fiszek).
    * `Flashcard` -> Obiekt z polami `id`, `text_front`, `text_back` oraz stanami postępu dla obu kierunków (`state_Front_to_Back`, `state_Back_to_Front`).
* **Relative Path:** System operuje na ścieżkach względnych względem `rootDir`, co zapewnia przenoszalność bazy danych między systemami.

## 4. Algorytmy i Przepływ Informacji (Data Flow)

### Algorytm Bezpiecznego Zapisu (SaveList):
1.  **Serializacja:** Obiekt `FlashcardList` jest konwertowany na strukturę `nlohmann::json` w pamięci RAM.
2.  **Zapis Tymczasowy:** Dane są zapisywane do pliku o nazwie `{nazwa}.json.tmp`.
3.  **Weryfikacja:** Zamknięcie strumienia i upewnienie się, że operacja zapisu powiodła się.
4.  **Zamiana (Atomic Rename):** Wywołanie `std::filesystem::rename` z pliku `.tmp` na docelowy `.json`. Jeśli krok zawiedzie, oryginalny plik pozostaje nienaruszony.

### Przepływ podczas Sesji Nauki:
1.  `DeckManager` wstrzykuje `IStorage` do domeny.
2.  UI wywołuje `loadList(path)`. `JsonStorage` parsuje plik i zwraca `std::shared_ptr<FlashcardList>`.
3.  **Modyfikacja treści:** Edycja fiszki w UI -> `DeckManager` natychmiast wywołuje `storage->saveList()`.
4.  **Aktualizacja stanu:** Zmiana `CardState` w trakcie sesji -> Zmiana w RAM (`shared_ptr`). Zapis na dysk następuje dopiero po zakończeniu sesji/powrocie do menu.

## 5. User Stories / Przypadki Użycia
* **US1 (Bezpieczeństwo):** Jako użytkownik, chcę mieć pewność, że jeśli mój komputer wyłączy się w trakcie zapisu, moja lista słówek nie zostanie nadpisana pustym lub uszkodzonym plikiem.
* **US2 (Internacjonalizacja):** Jako użytkownik uczący się języka koreańskiego, chcę, aby znaki specjalne (Hangul) były poprawnie zapisywane i odczytywane z plików JSON bez błędów kodowania.
* **US3 (Organizacja):** Jako użytkownik z dużą bazą danych, chcę organizować swoje listy w podfoldery (np. `Języki/Hiszpański/Czasowniki`), a system musi poprawnie zarządzać tymi ścieżkami na dysku.
* **US4 (Integralność):** Jako użytkownik, chcę zmienić nazwę folderu z moimi fiszkami, a system musi automatycznie zaktualizować lokalizację wszystkich zawartych w nim list, aby były widoczne w menu.