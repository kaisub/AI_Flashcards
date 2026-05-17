#pragma once

#include "app/localization/LocalizedText.hpp"
#include <string>

namespace app::localization::text {

namespace common {
inline constexpr LocalizedChar kStartSession{'s', 's'};
inline constexpr LocalizedChar kExitApp{'q', 'q'};
inline constexpr LocalizedChar kLanguageShortcut{'l', 'l'};
inline constexpr LocalizedCString kStartButton{"Start [S]", "Start [S]"};
inline constexpr LocalizedCString kLanguageButton{"Language [L]", "Język [L]"};
inline constexpr LocalizedCString kFront{"Front", "Przód"};
inline constexpr LocalizedCString kBack{"Back", "Tył"};
inline constexpr LocalizedCString kFrontLabel{"Front:", "Przód:"};
inline constexpr LocalizedCString kBackLabel{"Back:", "Tył:"};
inline constexpr LocalizedCString kSaveEnter{"Save [ENTER]", "Zapisz [ENTER]"};
inline constexpr LocalizedCString kCancelEscape{"Cancel [ESC]", "Anuluj [ESC]"};
inline constexpr LocalizedCString kCopyEnter{"Copy [ENTER]", "Kopiuj [ENTER]"};
inline constexpr LocalizedCString kBackEscape{"Exit [ESC]", "Wyjdź [ESC]"};
inline constexpr LocalizedCString kExitQ{"Exit [Q]", "Wyjdź [Q]"};
inline constexpr LocalizedCString kYesEnter{"Yes [ENTER]", "Tak [ENTER]"};
inline constexpr LocalizedCString kNoEscape{"No [ESC]", "Nie [ESC]"};
inline constexpr LocalizedCString kCreateEnter{"Create [ENTER]", "Utwórz [ENTER]"};
inline constexpr LocalizedCString kUnknown{"Unknown", "Nieznana"};
inline constexpr LocalizedCString kQuestionMark{"?", "?"};
inline constexpr LocalizedCString kStatusNew{"[Status: New]", "[Status: Nowa]"};
inline constexpr LocalizedCString kStatusKnown{"[Status: Known]", "[Status: Znana]"};
inline constexpr LocalizedCString kStatusMastered{"[Status: Mastered]", "[Status: Umiem]"};
}  // namespace common

namespace study_config {
inline constexpr LocalizedCString kDirectionFrontToBack{"Front -> Back", "Przód -> Tył"};
inline constexpr LocalizedCString kDirectionBackToFront{"Back -> Front", "Tył -> Przód"};
inline constexpr LocalizedCString kDirectionMixed{"Mixed", "Mieszany"};
inline constexpr LocalizedCString kOrderQueue{"Queue", "Kolejka"};
inline constexpr LocalizedCString kOrderRandom{"Random", "Losowo"};
inline constexpr LocalizedCString kModeStandard{"Standard", "Standardowy"};
inline constexpr LocalizedCString kModeFocused{"Focused", "Skupiony (Focused)"};
inline constexpr LocalizedCString kTargetNew{"New", "Nowe"};
inline constexpr LocalizedCString kTargetKnown{"Known", "Znane"};
inline constexpr LocalizedCString kTargetMastered{"Mastered", "Umiem"};
inline constexpr LocalizedCString kTitle{" Study Session Setup ", " Konfiguracja Sesji Nauki "};
inline constexpr LocalizedCString kDirectionLabel{" Direction:", " Kierunek:"};
inline constexpr LocalizedCString kOrderLabel{" Order:", " Porządek:"};
inline constexpr LocalizedCString kModeLabel{" Mode:", " Tryb:"};
inline constexpr LocalizedCString kTargetLabel{" Target (Focused only):", " Cel (Tylko Focused):"};
inline constexpr LocalizedCString kWeightsLabel{" Sampling Weights:", " Proporcje Wyświetlania:"};

inline std::string weightNewLabel(int weight) {
	return isEnglishLocale() ? " New (" + std::to_string(weight) + "%): " : " Nowe (" + std::to_string(weight) + "%): ";
}

inline std::string weightKnownLabel(int weight) {
	return isEnglishLocale() ? " Known (" + std::to_string(weight) + "%): " : " Znane (" + std::to_string(weight) + "%): ";
}

inline std::string weightMasteredLabel(int weight) {
	return isEnglishLocale() ? " Mastered (" + std::to_string(weight) + "%): " : " Umiem (" + std::to_string(weight) + "%): ";
}
}  // namespace study_config

namespace study_session {
inline constexpr LocalizedChar kUndoShortcut{'b', 'b'};
inline constexpr LocalizedChar kResetAllShortcut{'r', 'r'};
inline constexpr LocalizedChar kRateNewShortcut{'1', '1'};
inline constexpr LocalizedChar kRateKnownShortcut{'2', '2'};
inline constexpr LocalizedChar kRateMasteredShortcut{'3', '3'};
inline constexpr LocalizedChar kEditShortcut{'e', 'e'};
inline constexpr LocalizedChar kDeleteShortcut{'d', 'd'};
inline constexpr LocalizedChar kCopyShortcut{'c', 'c'};
inline constexpr LocalizedCString kFinishButton{"Finish", "Zakończ"};
inline constexpr LocalizedCString kCompleteTitle{"Session complete", "Sesja zakończona"};
inline constexpr LocalizedCString kDeletePrompt{"Are you sure you want to delete this flashcard?", "Czy na pewno chcesz usunąć tę fiszkę?"};
inline constexpr LocalizedCString kEditTitle{"Edit Card", "Edytuj Kartę"};
inline constexpr LocalizedCString kCopyTitle{"Copy Card To List", "Kopiuj Kartę do Listy"};
inline constexpr LocalizedCString kChooseListLabel{"Choose destination list:", "Wybierz listę docelową:"};
inline constexpr LocalizedCString kExitButton{"Exit [ESC]", "Wyjdź [ESC]"};
inline constexpr LocalizedCString kUndoButton{"Undo [ALT+B]", "Cofnij [ALT+B]"};
inline constexpr LocalizedCString kRateNewButton{"NEW [← | ALT+1]", "NOWA [← | ALT+1]"};
inline constexpr LocalizedCString kRateKnownButton{"KNOWN [↓ | ALT+2]", "ZNANA [↓ | ALT+2]"};
inline constexpr LocalizedCString kRateMasteredButton{"MASTERED [→ | ALT+3]", "UMIEM [→ | ALT+3]"};
inline constexpr LocalizedCString kAnswerPlaceholder{"Type your answer...", "Wpisz odpowiedź..."};
inline constexpr LocalizedCString kHintFlipButton{"[↑ / ENTER] Check answer and flip", "[↑ / ENTER] Sprawdź i odwróć"};
inline constexpr LocalizedCString kHintResetAllButton{"[ALT+R] Reset all", "[ALT+R] Resetuj wszystko"};
inline constexpr LocalizedCString kHintEditButton{"[ALT+E] Edit", "[ALT+E] Edytuj"};
inline constexpr LocalizedCString kHintDeleteButton{"[ALT+D] Delete", "[ALT+D] Usuń"};
inline constexpr LocalizedCString kHintCopyButton{"[ALT+C] Copy", "[ALT+C] Kopiuj"};
}  // namespace study_session

namespace deck_editor {
inline constexpr LocalizedChar kEditShortcut{'e', 'e'};
inline constexpr LocalizedChar kDeleteShortcut{'u', 'u'};
inline constexpr LocalizedChar kCopyShortcut{'k', 'k'};
inline constexpr LocalizedChar kMoveShortcut{'p', 'p'};
inline constexpr LocalizedChar kImportShortcut{'i', 'i'};
inline constexpr LocalizedChar kSelectAllShortcut{'a', 'a'};
inline constexpr LocalizedChar kDeselectAllShortcut{'d', 'd'};
inline constexpr LocalizedCString kEditTitle{"Edit Card", "Edytuj Fiszkę"};
inline constexpr LocalizedCString kDeleteConfirmButton{"Yes, Delete", "Tak, usuń"};
inline constexpr LocalizedCString kDeleteTitle{"Delete Selected Cards", "Usuń zaznaczone fiszki"};
inline constexpr LocalizedCString kMoveConfirmButton{"Move", "Przenieś"};
inline constexpr LocalizedCString kMoveTitle{"Move Selected Cards", "Przenieś zaznaczone fiszki"};
inline constexpr LocalizedCString kCopyTitle{"Copy Selected Cards", "Kopiuj zaznaczone fiszki"};
inline constexpr LocalizedCString kAddCardButton{"Add Card [ENTER]", "Dodaj fiszkę [ENTER]"};
inline constexpr LocalizedCString kSelectAllToolbarButton{"Select [A]", "Zaznacz wszystko [A]"};
inline constexpr LocalizedCString kDeselectAllToolbarButton{"Deselect [D]", "Odznacz wszystko [D]"};
inline constexpr LocalizedCString kStartSessionButton{"Start [S]", "Start [S]"};
inline constexpr LocalizedCString kDeleteToolbarButton{"Delete [U]", "Usuń [U]"};
inline constexpr LocalizedCString kCopyToolbarButton{"Copy [K]", "Kopiuj [K]"};
inline constexpr LocalizedCString kMoveToolbarButton{"Move [P]", "Przenieś [P]"};
inline constexpr LocalizedCString kEmptyDeck{"No cards in this deck yet.", "Brak fiszek w tej talii."};
inline constexpr LocalizedCString kSelectionHint{" [SPACE] Select | [E] Edit | [UP/DOWN] Navigate ", " [SPACJA] Zaznacz | [E] Edytuj | [↑/↓] Nawigacja "};
inline constexpr LocalizedCString kFrontFieldLabel{" Front: ", " Przód: "};
inline constexpr LocalizedCString kBackFieldLabel{" Back: ", " Tył: "};
inline constexpr LocalizedCString kImportTitle{" Import Cards ", " Importuj Fiszki "};
inline constexpr LocalizedCString kImportFilePathLabel{" File path: ", " Ścieżka pliku: "};
inline constexpr LocalizedCString kImportDelimiterLabel{" Delimiter:", " Separator:"};
inline constexpr LocalizedCString kImportCustomDelimiterLabel{" Custom delimiter: ", " Własny separator: "};
inline constexpr LocalizedCString kImportIgnoreHeader{"Ignore first row", "Ignoruj pierwszy wiersz"};
inline constexpr LocalizedCString kImportConfirmButton{"Import [ENTER]", "Importuj [ENTER]"};
inline constexpr LocalizedCString kImportToolbarButton{"Import [I]", "Importuj [I]"};
inline constexpr LocalizedCString kImportOptionOther{"Other", "Inny"};
inline constexpr LocalizedCString kSelectFileButton{"Choose file...", "Wybierz plik..."};
inline constexpr LocalizedCString kFilePickerTitle{" Select CSV/TXT File ", " Wybierz plik CSV/TXT "};

inline std::string headerTitle(const std::string& deckName) {
	return isEnglishLocale() ? " Deck Editor: " + deckName + " " : " Edytor Talii: " + deckName + " ";
}

inline std::string headerStats(size_t cardCount, size_t selectedCount) {
	return isEnglishLocale()
		? " Total Cards: " + std::to_string(cardCount) + " | Selected: " + std::to_string(selectedCount) + " "
		: " Wszystkie fiszki: " + std::to_string(cardCount) + " | Zaznaczone: " + std::to_string(selectedCount) + " ";
}

inline std::string deletePrompt(size_t selectedCount) {
	return isEnglishLocale()
		? "Are you sure you want to delete " + std::to_string(selectedCount) + " card(s)?"
		: "Czy na pewno chcesz usunąć " + std::to_string(selectedCount) + " fiszek?";
}

inline std::string movePrompt(size_t selectedCount) {
	return isEnglishLocale()
		? "Move " + std::to_string(selectedCount) + " card(s) to:"
		: "Przenieś " + std::to_string(selectedCount) + " fiszek do:";
}

inline std::string copyPrompt(size_t selectedCount) {
	return isEnglishLocale()
		? "Copy " + std::to_string(selectedCount) + " card(s) to:"
		: "Kopiuj " + std::to_string(selectedCount) + " fiszek do:";
}
}  // namespace deck_editor

namespace lists_browser {
inline constexpr LocalizedChar kNewFolderShortcut{'f', 'f'};
inline constexpr LocalizedChar kNewListShortcut{'n', 'n'};
inline constexpr LocalizedChar kRenameShortcut{'r', 'r'};
inline constexpr LocalizedChar kDeleteShortcut{'u', 'u'};
inline constexpr LocalizedChar kSettingsShortcut{'a', 'a'};
inline constexpr LocalizedCString kDirectoryPrefix{"[DIR]  ", "[DIR]  "};
inline constexpr LocalizedCString kFilePrefix{"[FILE] ", "[PLIK] "};
inline constexpr LocalizedCString kListNamePlaceholder{"List name", "Nazwa listy"};
inline constexpr LocalizedCString kFolderNamePlaceholder{"Folder name", "Nazwa folderu"};
inline constexpr LocalizedCString kRenamePlaceholder{"New name", "Nowa nazwa"};
inline constexpr LocalizedCString kRenameButton{"Rename [ENTER]", "Zmień [ENTER]"};
inline constexpr LocalizedCString kCreateListPrompt{"Enter the name of the new list:", "Podaj nazwę nowej listy:"};
inline constexpr LocalizedCString kCreateFolderPrompt{"Enter the name of the new folder:", "Podaj nazwę nowego folderu:"};
inline constexpr LocalizedCString kRenamePrompt{"Rename item:", "Zmień nazwę:"};
inline constexpr LocalizedCString kDeletePrompt{"Are you sure you want to delete:", "Czy na pewno chcesz usunąć:"};
inline constexpr LocalizedCString kNewFolderButton{"New Folder [F]", "Nowy Folder [F]"};
inline constexpr LocalizedCString kNewListButton{"New List [N]", "Nowa Lista [N]"};
inline constexpr LocalizedCString kRenameToolbarButton{"Rename [R]", "Zmień nazwę [R]"};
inline constexpr LocalizedCString kDeleteToolbarButton{"Delete [U]", "Usuń [U]"};
inline constexpr LocalizedCString kLanguageButton{"Language [L]", "Język [L]"};
inline constexpr LocalizedCString kSettingsButton{"Settings [A]", "Ustawienia [A]"};
inline constexpr LocalizedCString kSettingsDialogTitle{"Settings", "Ustawienia"};
inline constexpr LocalizedCString kMakeCopyButton{"Make Copy [C]", "Zrób kopię [C]"};
inline constexpr LocalizedCString kRestoreButton{"Restore [R]", "Przywróć [R]"};
inline constexpr LocalizedCString kBackupDialogTitle{"Create data backup", "Utwórz kopię folderu data"};
inline constexpr LocalizedCString kBackupTargetDirLabel{"Target folder:", "Folder docelowy:"};
inline constexpr LocalizedCString kBackupFileNameLabel{"File name (.zip):", "Nazwa pliku (.zip):"};
inline constexpr LocalizedCString kBackupFileNamePlaceholder{"flashcards_backup", "flashcards_backup"};
inline constexpr LocalizedCString kBackupBrowseButton{"Browse...", "Przeglądaj..."};
inline constexpr LocalizedCString kBackupPickerTitle{"Select backup folder", "Wybierz folder docelowy"};
inline constexpr LocalizedCString kBackupSelectFolderButton{"Select this folder [S]", "Wybierz ten folder [S]"};
inline constexpr LocalizedCString kBackupSaveButton{"Save backup [ENTER]", "Zapisz kopię [ENTER]"};
inline constexpr LocalizedCString kBackupSaved{"Backup created successfully.", "Kopia została utworzona."};
inline constexpr LocalizedCString kBackupSourceMissing{"Source folder 'data' does not exist.", "Folder źródłowy 'data' nie istnieje."};
inline constexpr LocalizedCString kBackupPathEmpty{"Provide a file name.", "Podaj nazwę pliku."};
inline constexpr LocalizedCString kBackupFailed{"Backup creation failed.", "Nie udało się utworzyć kopii."};
inline constexpr LocalizedCString kBackupOverwriteQuestion{"File already exists. Overwrite?", "Plik już istnieje. Nadpisać?"};
inline constexpr LocalizedCString kTitle{" AI Flashcards - Lists Browser ", " AI Flashcards - Przeglądarka List "};

inline std::string currentPath(const std::string& path) {
	return isEnglishLocale() ? " Current Path: " + path : " Bieżąca ścieżka: " + path;
}
}  // namespace lists_browser

}  // namespace app::localization::text