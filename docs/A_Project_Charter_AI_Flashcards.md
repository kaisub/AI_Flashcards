---
tags:
  - plan
  - architektura
  - system
  - lista
---
 ## 1. Project Goals (Cele Projektu)

- **Cel Osobisty:** Opanowanie nowoczesnego warsztatu AI, CI/CD, C++, integracji Pythona i architektury systemów.
    
- **Cel Użytkowy:** Stworzenie wydajnego, multiplatformowego systemu do nauki słówek metodą Spaced Repetition (powtórki w odstępach), z docelową integracją AI.
    

## 2. Scope Definition (Zakres MVP)

### 2.1 IN-SCOPE (Wchodzi w zakres)

- **Zarządzanie:** Pełne zarządzanie listami fiszek (dodaj, usuń, zmień nazwę, edytuj in-place, edytuj listę).
    
- **Struktura:** Hierarchia oparta na folderach na dysku zawierających listy fiszek.
    
- **Logika Nauki (Maszyna Stanów):** * Fiszka posiada 3 stany: _Nowa_, _Znana_, _Umiem_.
    
    - Podczas sesji fiszki trzymane są w 3 oddzielnych kolejkach.
        
    - System losuje fiszki z zachowaniem ustalonych, konfigurowalnych proporcji (np. 70% Nowe, 23% Znane, 7% Umiem).
        
    - Ocena użytkownika aktualizuje stan fiszki i przesuwa ją na koniec odpowiedniej kolejki.
        
- **Tryby Powtórek i Interakcja:** * Wybór kierunku tłumaczenia (Język 1 -> Język 2, Język 2 -> Język 1, tryb mieszany).
    
    - **Sortowanie:** Możliwość wyświetlania fiszek w kolejności ich definicji na liście (sekwencyjnie) lub całkowicie losowo.
        
    - **Zarządzanie w locie:** Możliwość usunięcia wyświetlanej aktualnie fiszki z listy lub skopiowania/przeniesienia jej do innej listy prosto z ekranu powtórki.
        
- **Przechowywanie Danych:** Zapis list w plikach **JSON** (biblioteka `nlohmann/json`), z informacją o stanie niezależnie dla każdego kierunku tłumaczenia.
    
- **Import:** Import fiszek z lokalnych plików **CSV** (`Front_język_1 ; Back_język_2`).
    
- **Backup (Eksport):** Tworzenie i przywracanie kopii zapasowej całej bazy użytkownika (struktura folderów i plików JSON) w postaci pojedynczego archiwum **ZIP**.
    
- **Interfejs i System:** User interface tekstowy (TUI) w terminalu (Windows/Ubuntu).
    
- **Infrastruktura:** Środowisko z podpiętym CI/CD (GitHub Actions, CMake, testy jednostkowe).
    
- **Architektura C++:** Bezwzględne oddzielenie logiki biznesowej od prezentacji i dostępu do danych (czyste interfejsy wirtualne), gotowe na portowanie do GUI/Androida.
    

### 2.2 OUT-OF-SCOPE (Poza zakresem - odłożone na później)

- [ ] Graficzny Interfejs Użytkownika (GUI).
    
- [ ] Port na Androida / Web (Quizlet).
    
- [ ] Synchronizacja w chmurze między urządzeniami.
    
- [ ] Generowanie grafik/dźwięków do fiszek za pomocą modeli AI.
    
- [ ] Bezpośrednia integracja z Google Sheets (import po publicznym linku API).
    
- [ ] Moduł generowania statystyk po zakończeniu sesji.
    

## 3. Constraints & Assumptions (Ograniczenia i Założenia)

- **Czas:** Praca w trybie "steady-pace" (według dostępności). Sterowanie pracą poprzez ścisłe zamykanie kamieni milowych i tasków na tablicy Kanban. Brak presji terminowej.
    
- **Jakość i UX:** Kod C++ bardzo wysokiej jakości (CI/CD, Code Review przez AI, testy jednostkowe na "Core"). Interfejs TUI musi być przede wszystkim prosty i czytelny (Form over function).
    
- **Tech Stack:** C++, Python, CMake, Docker, lokalne skrypty bash/powershell, Obsidian (Baza wiedzy / Kanban).
    
- **Cross-platform:** Kod musi kompilować się z użyciem CMake zarówno w środowisku Windows (np. MSVC/MinGW) jak i Linux (GCC/Clang).
    

## 4. Success Criteria (Kryteria Sukcesu MVP)

1. Tworzenie i zarządzanie folderami, listami i fiszkami (JSON) z poziomu TUI działa bezbłędnie.
    
2. Działa poprawny import fiszek z plików lokalnych CSV, a baza wiedzy daje się bezpiecznie spakować do archiwum ZIP i z niego odtworzyć.
    
3. Sesja powtórek uwzględnia tryb sekwencyjny i losowy, a interfejs pozwala na usunięcie fiszki lub wrzucenie jej do innej listy bezpośrednio z ekranu nauki.
    
4. Automatyzacja CI/CD: Każdy push do repozytorium na GitHubie uruchamia bezbłędny build i testy środowisku Ubuntu.
