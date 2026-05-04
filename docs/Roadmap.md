---
tags:
  - architektura
  - lista
  - plan
  - system
---
## Roadmapa Projektu: AI Flashcards (MVP)

### 🏗️ Milestone 0: Infrastruktura i Bootstrap (Środowisko)

Cel: Posiadanie gotowego, deterministycznego środowiska kompilacji i testowania.

**Task 0.0:** Środowisko: install wsl2 i ubuntu, docker desktop, VSC na windows + wtyczki "WSL" i "Dev Containers"

**Task 0.1:** Utworzenie struktury repozytorium Gita (src/, tests/, include/).

**Task 0.2:** Stworzenie pliku Dockerfile z bazowym środowiskiem Ubuntu (C++20, GCC/Clang, CMake, GTest).

**Task 0.3:** Konfiguracja systemu budowania CMakeLists.txt (podział na bibliotekę Core, plik wykonywalny i testy).

**Task 0.4:** Integracja środowiska w VS Code (np. Dev Containers lub konfiguracja WSL).

**Task 0.5:** Konfiguracja CI/CD - plik .github/workflows/build.yml (automatyczna kompilacja i odpalanie testów po pushu).

### 🧠 Milestone 1: Core Domain i Logika Biznesowa (C++ TDD)

Cel: Implementacja logiki fiszek i Maszyny Stanów BEZ wiedzy o plikach i UI. Tylko struktury danych i algorytmy w pamięci.

**Task 1.1:** Implementacja domeny: Klasa/Struktura Flashcard ze stanami (New, Known, Mastered).

**Task 1.2:** Implementacja klasy FlashcardList (kolekcja fiszek, dodawanie, usuwanie, edycja in-place).

**Task 1.3:** Implementacja algorytmu sesji nauki (StudySession): rozdzielanie do 3 kolejek, losowanie z zachowaniem proporcji (np. 70/23/7).

**Task 1.4:** Pokrycie logiki w 100% testami jednostkowymi (Google Test).

### 🐧 Milestone 1.5: Weryfikacja Cross-Platform (Native Ubuntu)
*Cel: Przeniesienie dotychczasowej pracy na natywny system Ubuntu i weryfikacja procesu.*
- [ ] **Task 1.5.1:** Sklonowanie repozytorium z GitHuba na natywnym systemie Ubuntu.
- [ ] **Task 1.5.2:** Uruchomienie kontenera Docker na natywnym Ubuntu i odpalenie testów jednostkowych.
- [ ] **Task 1.5.3:** Natywna kompilacja bez Dockera (lokalne GCC/CMake) w celu potwierdzenia niezależności od kontenerów.

### 💾 Milestone 2: Persystencja Danych (Warstwa Adapterów)

Cel: Zapis i odczyt z plików z zachowaniem abstrakcji (IStorage).

**Task 2.1:** Definicja interfejsu wirtualnego IStorage w domenie.

**Task 2.2:** Integracja biblioteki nlohmann/json poprzez system CMake (np. FetchContent).

**Task 2.3:** Implementacja klasy JsonStorage (dziedziczącej po IStorage), wykonującej zapis/odczyt plików z zachowaniem struktury folderów.

**Task 2.4:** Testy jednostkowe dla zapisu/odczytu JSON z wykorzystaniem plików tymczasowych.

### 🖥️ Milestone 3: Interfejs Użytkownika (TUI)

Cel: Podłączenie tekstowego interfejsu użytkownika, który komunikuje się z aplikacją przez warstwę kontrolera.

**Task 3.1:** Stworzenie architektury warstwy aplikacji (Aplikacja jako spoiwo między Core, Storage a UI).

**Task 3.2:** Zaprojektowanie głównej pętli zdarzeń aplikacji tekstowej.

**Task 3.3:** Widok: Zarządzanie listami (TUI).

**Task 3.4:** Widok: Pętla nauki / Tryb powtórek (kierunek, sortowanie, przenoszenie fiszek).

Wyodrębnione dwie fazy:
### ⚙️ Milestone 3A: Warstwa Aplikacji i Interfejsy Widoków (App Core)

**Cel:** Zaprojektowanie silnika napędzającego interfejs (kontrolera) oraz zdefiniowanie czysto wirtualnych kontraktów (interfejsów) dla ekranów. Żadnego FTXUI na tym etapie.

1. **Zdefiniowanie `IView`:** Stworzenie wirtualnych klas dla głównych ekranów (np. `IMainMenuView`, `IStudySessionView`).
    
2. **Implementacja `AppController`:** Klasa zarządzająca pętlą główną, przechowująca `DeckManager` i przełączająca stany (State Machine).
    
3. **Mockowanie i Testy:** Stworzenie `DummyViews` (np. takich, które tylko wypisują logi w `std::cout` lub używają GMock) i pokrycie `AppController` testami jednostkowymi.
### 🖥️ Milestone 3B: Implementacja FTXUI (Concrete Views)

**Cel:** Tchnięcie życia w stworzone interfejsy za pomocą gotowej biblioteki TUI.

1. **Integracja:** Dodanie biblioteki FTXUI do `CMakeLists.txt`.
    
2. **Implementacja Widoków:** Utworzenie klas takich jak `FtxuiStudySessionView` dziedziczących po `IStudySessionView`, które renderują odpowiednie komponenty (przyciski, teksty, ramki).
    
3. **Podpięcie do Kontrolera:** W funkcji `main.cpp` wstrzyknięcie widoków FTXUI do naszego `AppController`.
### 🔄 Milestone 4: Integracje i Narzędzia Zewnętrzne (Import/Export)

Cel: Obsługa formatów zewnętrznych i bezpieczeństwo danych użytkownika.

**Task 4.1:** Implementacja parsera plików CSV (import).

**Task 4.2:** Integracja narzędzia ZIP w C++ (lub za pomocą skryptu systemowego wywoływanego z C++) dla tworzenia backupów.

**Task 4.3:** Dodanie opcji Import/Export do menu TUI.

### 🤖 Milestone 5: Warsztat AI (Python & RAG Prep) - Równoległy

Cel: Stworzenie skryptów usprawniających pracę i kładących podwaliny pod RAG.

**Task 5.1:** Stworzenie skryptów bash/powershell do lokalnego uruchamiania agenta Gemini CLI jako lintera dla C++.

**Task 5.2:** (Python) Napisanie małego skryptu parsującego bazę Obsidian (odczyt struktury nagłówków .md), jako Proof of Concept pod przyszłą bazę wektorową.
