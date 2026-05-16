### 1. Cele i Architektura Systemu

- **Import słówek (CSV/TXT):** Zasilanie istniejącej listy nowymi fiszkami. Wykorzystanie header-only biblioteki `rapidcsv` do parsowania.
- **Backup / Restore (ZIP):** Bezpieczne tworzenie i odtwarzanie kopii zapasowej katalogu `data/`. Wykorzystanie header-only biblioteki `miniz` / `miniz-cpp`.
- **Niezawodność (Restore):** Mechanizm _Safe Swap_ – wypakowanie archiwum, a w przypadku sukcesu podmienienie folderu systemowego (wykorzystanie `data_tmp/`).
- **Obsługa Błędów:** Wszelkie błędy wejścia/wyjścia (I/O) muszą być przechwytywane i prezentowane w GUI za pomocą uniwersalnego okna dialogowego (Error Modal).

### 2. Logika Domenowa (Core)

- **`FlashcardList` / `DeckManager` (Import):**
    - Odczyt pliku wiersz po wierszu z uwzględnieniem konfigurowalnego separatora (`,`, `;`, `\t`, `własny`).
    - Opcjonalne pomijanie pierwszego wiersza (nagłówków).
    - Automatyczne ignorowanie znaków BOM (Byte Order Mark) z początku pliku.
    - Ignorowanie wadliwych wierszy (inna liczba kolumn niż 2). Akceptacja duplikatów.
- **`IStorage` / `JsonStorage` (Backup):**
    - `exportToZip(targetPath)`: Kompresja całego folderu `data/` do wskazanego archiwum.
    - `restoreFromZip(sourcePath)`: Zmiana nazwy `data/` na `data_tmp/`, rozpakowanie `sourcePath` do `data/`. W przypadku poprawnego rozpakowania – usunięcie `data_tmp/`. W przypadku błędu – usunięcie błędnego `data/`, przywrócenie nazwy z `data_tmp/` i wyrzucenie wyjątku.

### 3. Specyfikacja Interfejsu Użytkownika (FTXUI)

- **Dialog Błędów (Wspólny):** Uniwersalny Modal (Tytuł, Treść błędu, `[OK]`), blokujący interfejs do momentu kliknięcia.
- **UI Importu (w `FtxuiDeckEditorView`):**
    - Nowy przycisk: `Importuj (I)` w głównym pasku narzędzi.
    - Modal:
        - `Input`: Ścieżka do pliku.
        - `Radiobox`: Separator (Przecinek, Średnik, Tabulator, Inny).
        - `Input`: Własny separator (widoczne/aktywne tylko dla "Inny").
        - `Checkbox`: `[ ] Ignoruj pierwszy wiersz (nagłówki)`.
    - Po sukcesie powrót do edytora z odświeżoną listą. W razie błędu – Modal z błędem.
    - Wpisywanie ścieżek z palca w konsoli to archaizm i proszenie się o błędy w literówkach. Potrzebujemy **TUI File Pickera** (wizualnego nawigatora po folderach).
- **UI Backup'u (w `FtxuiListsBrowserView`):**
    - Nowe przyciski: `Kopia Zapasowa (B)` oraz `Przywróć (R)`.
    - Modal dla Kopii: `Input` ze ścieżką (domyślnie sugerowane np. `backup.zip`).
    - Modal dla Przywracania: `Input` ze ścieżką do ZIP oraz tekst ostrzegawczy.