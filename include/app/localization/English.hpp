#pragma once

#include <string>

namespace app::localization::en {

namespace common {
inline constexpr char kStartSession = 's';
inline constexpr const char* kStartButton = "Start (S)";
inline constexpr const char* kFront = "Front";
inline constexpr const char* kBack = "Back";
inline constexpr const char* kFrontLabel = "Front:";
inline constexpr const char* kBackLabel = "Back:";
inline constexpr const char* kSaveEnter = "Save [ENTER]";
inline constexpr const char* kCancelEscape = "Cancel [ESC]";
inline constexpr const char* kCopyEnter = "Copy [ENTER]";
inline constexpr const char* kBackEscape = "Back (ESC)";
inline constexpr const char* kExitQ = "Exit (Q)";
inline constexpr const char* kYesEnter = "Yes [ENTER]";
inline constexpr const char* kNoEscape = "No [ESC]";
inline constexpr const char* kCreateEnter = "Create [ENTER]";
inline constexpr const char* kUnknown = "Unknown";
inline constexpr const char* kQuestionMark = "?";
inline constexpr const char* kStatusNew = "[Status: New]";
inline constexpr const char* kStatusKnown = "[Status: Known]";
inline constexpr const char* kStatusMastered = "[Status: Mastered]";
}  // namespace common

namespace study_config {
inline constexpr const char* kDirectionFrontToBack = "Front -> Back";
inline constexpr const char* kDirectionBackToFront = "Back -> Front";
inline constexpr const char* kDirectionMixed = "Mixed";
inline constexpr const char* kModeStandard = "Standard";
inline constexpr const char* kModeFocused = "Focused";
inline constexpr const char* kTargetNew = "New";
inline constexpr const char* kTargetKnown = "Known";
inline constexpr const char* kTargetMastered = "Mastered";
inline constexpr const char* kStartButton = common::kStartButton;
inline constexpr const char* kCancelButton = "Back (ESC)";
inline constexpr const char* kTitle = " Study Session Setup ";
inline constexpr const char* kDirectionLabel = " Direction:";
inline constexpr const char* kModeLabel = " Mode:";
inline constexpr const char* kTargetLabel = " Target (Focused only):";
inline constexpr const char* kWeightsLabel = " Sampling Weights:";

inline std::string weightNewLabel(int weight) {
    return " New (" + std::to_string(weight) + "%): ";
}

inline std::string weightKnownLabel(int weight) {
    return " Known (" + std::to_string(weight) + "%): ";
}

inline std::string weightMasteredLabel(int weight) {
    return " Mastered (" + std::to_string(weight) + "%): ";
}
}  // namespace study_config

namespace study_session {
inline constexpr const char* kFinishButton = "Finish";
inline constexpr const char* kCompleteTitle = "Session complete";
inline constexpr const char* kDeletePrompt = "Are you sure you want to delete this flashcard?";
inline constexpr const char* kEditTitle = "Edit Card";
inline constexpr const char* kCopyTitle = "Copy Card To List";
inline constexpr const char* kChooseListLabel = "Choose destination list:";
inline constexpr const char* kExitButton = "Exit (ESC)";
inline constexpr const char* kUndoButton = "Undo (ALT+B)";
inline constexpr const char* kRateNewButton = "NEW (LEFT | ALT+1)";
inline constexpr const char* kRateKnownButton = "KNOWN (DOWN | ALT+2)";
inline constexpr const char* kRateMasteredButton = "MASTERED (RIGHT | ALT+3)";
inline constexpr const char* kAnswerPlaceholder = "Type your answer...";
inline constexpr const char* kHintFlipButton = "(UP / ENTER) Check answer and flip";
inline constexpr const char* kHintResetAllButton = "(ALT+R) Reset all";
inline constexpr const char* kHintEditButton = "[ALT+E] Edit";
inline constexpr const char* kHintDeleteButton = "[ALT+D] Delete";
inline constexpr const char* kHintCopyButton = "[ALT+C] Copy";
}  // namespace study_session

namespace deck_editor {
inline constexpr const char* kEditTitle = "Edit Card";
inline constexpr const char* kDeleteConfirmButton = "Yes, Delete";
inline constexpr const char* kDeleteTitle = "Delete Selected Cards";
inline constexpr const char* kMoveConfirmButton = "Move";
inline constexpr const char* kMoveTitle = "Move Selected Cards";
inline constexpr const char* kCopyTitle = "Copy Selected Cards";
inline constexpr const char* kAddCardButton = "Add Card [ENTER]";
inline constexpr const char* kStartSessionButton = common::kStartButton;
inline constexpr const char* kDeleteToolbarButton = "Delete (U)";
inline constexpr const char* kCopyToolbarButton = "Copy (K)";
inline constexpr const char* kMoveToolbarButton = "Move (P)";
inline constexpr const char* kEmptyDeck = "No cards in this deck yet.";
inline constexpr const char* kSelectionHint = " [SPACE] Select | [E] Edit | [UP/DOWN] Navigate ";
inline constexpr const char* kFrontFieldLabel = " Front: ";
inline constexpr const char* kBackFieldLabel = " Back: ";
inline constexpr const char* kImportTitle = " Import Cards ";
inline constexpr const char* kImportFilePathLabel = " File path: ";
inline constexpr const char* kImportDelimiterLabel = " Delimiter:";
inline constexpr const char* kImportCustomDelimiterLabel = " Custom delimiter: ";
inline constexpr const char* kImportIgnoreHeader = "Ignore first row";
inline constexpr const char* kImportConfirmButton = "Import [ENTER]";
inline constexpr const char* kImportToolbarButton = "Import (I)";
inline constexpr const char* kImportOptionOther = "Other";
inline constexpr const char* kSelectFileButton = "Choose file...";
inline constexpr const char* kFilePickerTitle = " Select CSV/TXT File ";

inline std::string headerTitle(const std::string& deckName) {
    return " Deck Editor: " + deckName + " ";
}

inline std::string headerStats(size_t cardCount, size_t selectedCount) {
    return " Total Cards: " + std::to_string(cardCount) + " | Selected: " + std::to_string(selectedCount) + " ";
}

inline std::string deletePrompt(size_t selectedCount) {
    return "Are you sure you want to delete " + std::to_string(selectedCount) + " card(s)?";
}

inline std::string movePrompt(size_t selectedCount) {
    return "Move " + std::to_string(selectedCount) + " card(s) to:";
}

inline std::string copyPrompt(size_t selectedCount) {
    return "Copy " + std::to_string(selectedCount) + " card(s) to:";
}
}  // namespace deck_editor

namespace lists_browser {
inline constexpr const char* kDirectoryPrefix = "[DIR]  ";
inline constexpr const char* kFilePrefix = "[FILE] ";
inline constexpr const char* kListNamePlaceholder = "List name";
inline constexpr const char* kFolderNamePlaceholder = "Folder name";
inline constexpr const char* kRenamePlaceholder = "New name";
inline constexpr const char* kRenameButton = "Rename [ENTER]";
inline constexpr const char* kCreateListPrompt = "Enter the name of the new list:";
inline constexpr const char* kCreateFolderPrompt = "Enter the name of the new folder:";
inline constexpr const char* kRenamePrompt = "Rename item:";
inline constexpr const char* kDeletePrompt = "Are you sure you want to delete:";
inline constexpr const char* kNewFolderButton = "New Folder (F)";
inline constexpr const char* kNewListButton = "New List (N)";
inline constexpr const char* kRenameToolbarButton = "Rename (R)";
inline constexpr const char* kDeleteToolbarButton = "Delete (U)";
inline constexpr const char* kTitle = " AI Flashcards - Lists Browser ";

inline std::string currentPath(const std::string& path) {
    return " Current Path: " + path;
}
}  // namespace lists_browser

}  // namespace app::localization::en