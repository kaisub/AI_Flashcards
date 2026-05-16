#pragma once

#include <string>

namespace app::localization::pl {

namespace common {
inline constexpr char kStartSession = 's';
inline constexpr const char* kStartButton = "Start (S)";
inline constexpr const char* kFront = "Przód";
inline constexpr const char* kBack = "Tył";
inline constexpr const char* kFrontLabel = "Przód:";
inline constexpr const char* kBackLabel = "Tył:";
inline constexpr const char* kSaveEnter = "Zapisz [ENTER]";
inline constexpr const char* kCancelEscape = "Anuluj [ESC]";
inline constexpr const char* kCopyEnter = "Kopiuj [ENTER]";
inline constexpr const char* kBackEscape = "Wróć (ESC)";
inline constexpr const char* kExitQ = "Wyjdź (Q)";
inline constexpr const char* kYesEnter = "Tak [ENTER]";
inline constexpr const char* kNoEscape = "Nie [ESC]";
inline constexpr const char* kCreateEnter = "Utwórz [ENTER]";
inline constexpr const char* kUnknown = "Nieznana";
inline constexpr const char* kQuestionMark = "?";
inline constexpr const char* kStatusNew = "[Status: Nowa]";
inline constexpr const char* kStatusKnown = "[Status: Znana]";
inline constexpr const char* kStatusMastered = "[Status: Umiem]";
}  // namespace common

namespace study_config {
inline constexpr const char* kDirectionFrontToBack = "Przód -> Tył";
inline constexpr const char* kDirectionBackToFront = "Tył -> Przód";
inline constexpr const char* kDirectionMixed = "Mieszany";
inline constexpr const char* kModeStandard = "Standardowy";
inline constexpr const char* kModeFocused = "Skupiony (Focused)";
inline constexpr const char* kTargetNew = "Nowe";
inline constexpr const char* kTargetKnown = "Znane";
inline constexpr const char* kTargetMastered = "Umiem";
inline constexpr const char* kStartButton = common::kStartButton;
inline constexpr const char* kCancelButton = "Wróć (ESC)";
inline constexpr const char* kTitle = " Konfiguracja Sesji Nauki ";
inline constexpr const char* kDirectionLabel = " Kierunek:";
inline constexpr const char* kModeLabel = " Tryb:";
inline constexpr const char* kTargetLabel = " Cel (Tylko Focused):";
inline constexpr const char* kWeightsLabel = " Proporcje Losowania (Wagi):";

inline std::string weightNewLabel(int weight) {
    return " Nowe (" + std::to_string(weight) + "%): ";
}

inline std::string weightKnownLabel(int weight) {
    return " Znane (" + std::to_string(weight) + "%): ";
}

inline std::string weightMasteredLabel(int weight) {
    return " Umiem (" + std::to_string(weight) + "%): ";
}
}  // namespace study_config

namespace study_session {
inline constexpr const char* kFinishButton = "Zakończ";
inline constexpr const char* kCompleteTitle = "Sesja zakończona";
inline constexpr const char* kDeletePrompt = "Czy na pewno chcesz usunąć tę fiszkę?";
inline constexpr const char* kEditTitle = "Edytuj Kartę";
inline constexpr const char* kCopyTitle = "Kopiuj Kartę do Listy";
inline constexpr const char* kChooseListLabel = "Wybierz listę docelową:";
inline constexpr const char* kExitButton = "Wyjdź (ESC)";
inline constexpr const char* kUndoButton = "Cofnij (ALT+B)";
inline constexpr const char* kRateNewButton = "NOWA (← | ALT+1)";
inline constexpr const char* kRateKnownButton = "ZNANA (↓ | ALT+2)";
inline constexpr const char* kRateMasteredButton = "UMIEM (→ | ALT+3)";
inline constexpr const char* kAnswerPlaceholder = "Wpisz odpowiedź...";
inline constexpr const char* kHintFlipButton = "(↑ / ENTER) Sprawdź i odwróć";
inline constexpr const char* kHintResetAllButton = "(ALT+R) Reset all";
inline constexpr const char* kHintEditButton = "[ALT+E] Edytuj";
inline constexpr const char* kHintDeleteButton = "[ALT+D] Usuń";
inline constexpr const char* kHintCopyButton = "[ALT+C] Kopiuj";
}  // namespace study_session

namespace deck_editor {
inline constexpr const char* kEditTitle = "Edytuj Fiszkę";
inline constexpr const char* kDeleteConfirmButton = "Tak, usuń";
inline constexpr const char* kDeleteTitle = "Usuń zaznaczone fiszki";
inline constexpr const char* kMoveConfirmButton = "Przenieś";
inline constexpr const char* kMoveTitle = "Przenieś zaznaczone fiszki";
inline constexpr const char* kCopyTitle = "Kopiuj zaznaczone fiszki";
inline constexpr const char* kAddCardButton = "Dodaj fiszkę [ENTER]";
inline constexpr const char* kStartSessionButton = common::kStartButton;
inline constexpr const char* kDeleteToolbarButton = "Usuń (U)";
inline constexpr const char* kCopyToolbarButton = "Kopiuj (K)";
inline constexpr const char* kMoveToolbarButton = "Przenieś (P)";
inline constexpr const char* kEmptyDeck = "Brak fiszek w tej talii.";
inline constexpr const char* kSelectionHint = " [SPACJA] Zaznacz | [E] Edytuj | [↑/↓] Nawigacja ";
inline constexpr const char* kFrontFieldLabel = " Przód: ";
inline constexpr const char* kBackFieldLabel = " Tył: ";
inline constexpr const char* kImportTitle = " Importuj Fiszki ";
inline constexpr const char* kImportFilePathLabel = " Ścieżka pliku: ";
inline constexpr const char* kImportDelimiterLabel = " Separator:";
inline constexpr const char* kImportCustomDelimiterLabel = " Własny separator: ";
inline constexpr const char* kImportIgnoreHeader = "Ignoruj pierwszy wiersz";
inline constexpr const char* kImportConfirmButton = "Importuj [ENTER]";
inline constexpr const char* kImportToolbarButton = "Importuj (I)";
inline constexpr const char* kImportOptionOther = "Inny";
inline constexpr const char* kSelectFileButton = "Wybierz plik...";
inline constexpr const char* kFilePickerTitle = " Wybierz plik CSV/TXT ";

inline std::string headerTitle(const std::string& deckName) {
    return " Edytor Talii: " + deckName + " ";
}

inline std::string headerStats(size_t cardCount, size_t selectedCount) {
    return " Wszystkie fiszki: " + std::to_string(cardCount) + " | Zaznaczone: " + std::to_string(selectedCount) + " ";
}

inline std::string deletePrompt(size_t selectedCount) {
    return "Czy na pewno chcesz usunąć " + std::to_string(selectedCount) + " fiszek?";
}

inline std::string movePrompt(size_t selectedCount) {
    return "Przenieś " + std::to_string(selectedCount) + " fiszek do:";
}

inline std::string copyPrompt(size_t selectedCount) {
    return "Kopiuj " + std::to_string(selectedCount) + " fiszek do:";
}
}  // namespace deck_editor

namespace lists_browser {
inline constexpr const char* kDirectoryPrefix = "[DIR]  ";
inline constexpr const char* kFilePrefix = "[PLIK] ";
inline constexpr const char* kListNamePlaceholder = "Nazwa listy";
inline constexpr const char* kFolderNamePlaceholder = "Nazwa folderu";
inline constexpr const char* kRenamePlaceholder = "Nowa nazwa";
inline constexpr const char* kRenameButton = "Zmień [ENTER]";
inline constexpr const char* kCreateListPrompt = "Podaj nazwę nowej listy:";
inline constexpr const char* kCreateFolderPrompt = "Podaj nazwę nowego folderu:";
inline constexpr const char* kRenamePrompt = "Zmień nazwę:";
inline constexpr const char* kDeletePrompt = "Czy na pewno chcesz usunąć:";
inline constexpr const char* kNewFolderButton = "Nowy Folder (F)";
inline constexpr const char* kNewListButton = "Nowa Lista (N)";
inline constexpr const char* kRenameToolbarButton = "Zmień nazwę (R)";
inline constexpr const char* kDeleteToolbarButton = "Usuń (U)";
inline constexpr const char* kTitle = " AI Flashcards - Przeglądarka List ";

inline std::string currentPath(const std::string& path) {
    return " Bieżąca ścieżka: " + path;
}
}  // namespace lists_browser

}  // namespace app::localization::pl