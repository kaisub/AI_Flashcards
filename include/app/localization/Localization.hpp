#pragma once

#include "app/localization/English.hpp"
#include "app/localization/Polish.hpp"
#include <ftxui/util/ref.hpp>
#include <string>

namespace app::localization {

enum class Locale {
	Polish,
	English,
};

#if defined(AI_FLASHCARDS_LOCALE_PL)
inline constexpr Locale kDefaultLocale = Locale::Polish;
#elif defined(AI_FLASHCARDS_LOCALE_EN)
inline constexpr Locale kDefaultLocale = Locale::English;
#else
#error "No UI locale selected. Define AI_FLASHCARDS_LOCALE_PL or AI_FLASHCARDS_LOCALE_EN."
#endif

inline Locale& localeStorage() {
	static Locale locale = kDefaultLocale;
	return locale;
}

inline Locale currentLocale() {
	return localeStorage();
}

inline void setCurrentLocale(Locale locale) {
	localeStorage() = locale;
}

inline void toggleLocale() {
	localeStorage() = (localeStorage() == Locale::Polish) ? Locale::English : Locale::Polish;
}

inline bool isEnglishLocale() {
	return currentLocale() == Locale::English;
}

struct RuntimeCString {
	const char* (*getter)();

	operator std::string() const {
		return getter();
	}

	operator ftxui::ConstStringRef() const {
		return ftxui::ConstStringRef(getter());
	}

	operator ftxui::StringRef() const {
		return ftxui::StringRef(getter());
	}

	std::string str() const {
		return getter();
	}

	bool starts_with(const std::string& prefix) const {
		return str().starts_with(prefix);
	}

	bool starts_with(const char* prefix) const {
		return str().starts_with(prefix);
	}
};

struct RuntimeChar {
	char (*getter)();

	constexpr operator char() const {
		return getter();
	}
};

#define APP_LOCALIZED_CSTRING(NS, NAME) \
	inline const char* get_##NS##_##NAME() { return isEnglishLocale() ? en::NS::NAME : pl::NS::NAME; } \
	inline constexpr RuntimeCString NAME{get_##NS##_##NAME}

#define APP_LOCALIZED_CHAR(NS, NAME) \
	inline char get_##NS##_##NAME() { return isEnglishLocale() ? en::NS::NAME : pl::NS::NAME; } \
	inline constexpr RuntimeChar NAME{get_##NS##_##NAME}

namespace runtime {

namespace common {
APP_LOCALIZED_CHAR(common, kStartSession);
APP_LOCALIZED_CHAR(common, kExitApp);
APP_LOCALIZED_CHAR(common, kLanguageShortcut);
APP_LOCALIZED_CSTRING(common, kStartButton);
APP_LOCALIZED_CSTRING(common, kLanguageButton);
APP_LOCALIZED_CSTRING(common, kFront);
APP_LOCALIZED_CSTRING(common, kBack);
APP_LOCALIZED_CSTRING(common, kFrontLabel);
APP_LOCALIZED_CSTRING(common, kBackLabel);
APP_LOCALIZED_CSTRING(common, kSaveEnter);
APP_LOCALIZED_CSTRING(common, kCancelEscape);
APP_LOCALIZED_CSTRING(common, kCopyEnter);
APP_LOCALIZED_CSTRING(common, kBackEscape);
APP_LOCALIZED_CSTRING(common, kExitQ);
APP_LOCALIZED_CSTRING(common, kYesEnter);
APP_LOCALIZED_CSTRING(common, kNoEscape);
APP_LOCALIZED_CSTRING(common, kCreateEnter);
APP_LOCALIZED_CSTRING(common, kUnknown);
APP_LOCALIZED_CSTRING(common, kQuestionMark);
APP_LOCALIZED_CSTRING(common, kStatusNew);
APP_LOCALIZED_CSTRING(common, kStatusKnown);
APP_LOCALIZED_CSTRING(common, kStatusMastered);
} // namespace common

namespace study_config {
APP_LOCALIZED_CSTRING(study_config, kDirectionFrontToBack);
APP_LOCALIZED_CSTRING(study_config, kDirectionBackToFront);
APP_LOCALIZED_CSTRING(study_config, kDirectionMixed);
APP_LOCALIZED_CSTRING(study_config, kModeStandard);
APP_LOCALIZED_CSTRING(study_config, kModeFocused);
APP_LOCALIZED_CSTRING(study_config, kTargetNew);
APP_LOCALIZED_CSTRING(study_config, kTargetKnown);
APP_LOCALIZED_CSTRING(study_config, kTargetMastered);
APP_LOCALIZED_CSTRING(study_config, kTitle);
APP_LOCALIZED_CSTRING(study_config, kDirectionLabel);
APP_LOCALIZED_CSTRING(study_config, kModeLabel);
APP_LOCALIZED_CSTRING(study_config, kTargetLabel);
APP_LOCALIZED_CSTRING(study_config, kWeightsLabel);

inline std::string weightNewLabel(int weight) {
	return isEnglishLocale() ? en::study_config::weightNewLabel(weight) : pl::study_config::weightNewLabel(weight);
}

inline std::string weightKnownLabel(int weight) {
	return isEnglishLocale() ? en::study_config::weightKnownLabel(weight) : pl::study_config::weightKnownLabel(weight);
}

inline std::string weightMasteredLabel(int weight) {
	return isEnglishLocale() ? en::study_config::weightMasteredLabel(weight) : pl::study_config::weightMasteredLabel(weight);
}
} // namespace study_config

namespace study_session {
APP_LOCALIZED_CHAR(study_session, kUndoShortcut);
APP_LOCALIZED_CHAR(study_session, kResetAllShortcut);
APP_LOCALIZED_CHAR(study_session, kRateNewShortcut);
APP_LOCALIZED_CHAR(study_session, kRateKnownShortcut);
APP_LOCALIZED_CHAR(study_session, kRateMasteredShortcut);
APP_LOCALIZED_CHAR(study_session, kEditShortcut);
APP_LOCALIZED_CHAR(study_session, kDeleteShortcut);
APP_LOCALIZED_CHAR(study_session, kCopyShortcut);
APP_LOCALIZED_CSTRING(study_session, kFinishButton);
APP_LOCALIZED_CSTRING(study_session, kCompleteTitle);
APP_LOCALIZED_CSTRING(study_session, kDeletePrompt);
APP_LOCALIZED_CSTRING(study_session, kEditTitle);
APP_LOCALIZED_CSTRING(study_session, kCopyTitle);
APP_LOCALIZED_CSTRING(study_session, kChooseListLabel);
APP_LOCALIZED_CSTRING(study_session, kExitButton);
APP_LOCALIZED_CSTRING(study_session, kUndoButton);
APP_LOCALIZED_CSTRING(study_session, kRateNewButton);
APP_LOCALIZED_CSTRING(study_session, kRateKnownButton);
APP_LOCALIZED_CSTRING(study_session, kRateMasteredButton);
APP_LOCALIZED_CSTRING(study_session, kAnswerPlaceholder);
APP_LOCALIZED_CSTRING(study_session, kHintFlipButton);
APP_LOCALIZED_CSTRING(study_session, kHintResetAllButton);
APP_LOCALIZED_CSTRING(study_session, kHintEditButton);
APP_LOCALIZED_CSTRING(study_session, kHintDeleteButton);
APP_LOCALIZED_CSTRING(study_session, kHintCopyButton);
} // namespace study_session

namespace deck_editor {
APP_LOCALIZED_CHAR(deck_editor, kEditShortcut);
APP_LOCALIZED_CHAR(deck_editor, kDeleteShortcut);
APP_LOCALIZED_CHAR(deck_editor, kCopyShortcut);
APP_LOCALIZED_CHAR(deck_editor, kMoveShortcut);
APP_LOCALIZED_CHAR(deck_editor, kImportShortcut);
APP_LOCALIZED_CSTRING(deck_editor, kEditTitle);
APP_LOCALIZED_CSTRING(deck_editor, kDeleteConfirmButton);
APP_LOCALIZED_CSTRING(deck_editor, kDeleteTitle);
APP_LOCALIZED_CSTRING(deck_editor, kMoveConfirmButton);
APP_LOCALIZED_CSTRING(deck_editor, kMoveTitle);
APP_LOCALIZED_CSTRING(deck_editor, kCopyTitle);
APP_LOCALIZED_CSTRING(deck_editor, kAddCardButton);
APP_LOCALIZED_CSTRING(deck_editor, kStartSessionButton);
APP_LOCALIZED_CSTRING(deck_editor, kDeleteToolbarButton);
APP_LOCALIZED_CSTRING(deck_editor, kCopyToolbarButton);
APP_LOCALIZED_CSTRING(deck_editor, kMoveToolbarButton);
APP_LOCALIZED_CSTRING(deck_editor, kEmptyDeck);
APP_LOCALIZED_CSTRING(deck_editor, kSelectionHint);
APP_LOCALIZED_CSTRING(deck_editor, kFrontFieldLabel);
APP_LOCALIZED_CSTRING(deck_editor, kBackFieldLabel);
APP_LOCALIZED_CSTRING(deck_editor, kImportTitle);
APP_LOCALIZED_CSTRING(deck_editor, kImportFilePathLabel);
APP_LOCALIZED_CSTRING(deck_editor, kImportDelimiterLabel);
APP_LOCALIZED_CSTRING(deck_editor, kImportCustomDelimiterLabel);
APP_LOCALIZED_CSTRING(deck_editor, kImportIgnoreHeader);
APP_LOCALIZED_CSTRING(deck_editor, kImportConfirmButton);
APP_LOCALIZED_CSTRING(deck_editor, kImportToolbarButton);
APP_LOCALIZED_CSTRING(deck_editor, kImportOptionOther);
APP_LOCALIZED_CSTRING(deck_editor, kSelectFileButton);
APP_LOCALIZED_CSTRING(deck_editor, kFilePickerTitle);

inline std::string headerTitle(const std::string& deckName) {
	return isEnglishLocale() ? en::deck_editor::headerTitle(deckName) : pl::deck_editor::headerTitle(deckName);
}

inline std::string headerStats(size_t cardCount, size_t selectedCount) {
	return isEnglishLocale() ? en::deck_editor::headerStats(cardCount, selectedCount) : pl::deck_editor::headerStats(cardCount, selectedCount);
}

inline std::string deletePrompt(size_t selectedCount) {
	return isEnglishLocale() ? en::deck_editor::deletePrompt(selectedCount) : pl::deck_editor::deletePrompt(selectedCount);
}

inline std::string movePrompt(size_t selectedCount) {
	return isEnglishLocale() ? en::deck_editor::movePrompt(selectedCount) : pl::deck_editor::movePrompt(selectedCount);
}

inline std::string copyPrompt(size_t selectedCount) {
	return isEnglishLocale() ? en::deck_editor::copyPrompt(selectedCount) : pl::deck_editor::copyPrompt(selectedCount);
}
} // namespace deck_editor

namespace lists_browser {
APP_LOCALIZED_CHAR(lists_browser, kNewFolderShortcut);
APP_LOCALIZED_CHAR(lists_browser, kNewListShortcut);
APP_LOCALIZED_CHAR(lists_browser, kRenameShortcut);
APP_LOCALIZED_CHAR(lists_browser, kDeleteShortcut);
APP_LOCALIZED_CSTRING(lists_browser, kDirectoryPrefix);
APP_LOCALIZED_CSTRING(lists_browser, kFilePrefix);
APP_LOCALIZED_CSTRING(lists_browser, kListNamePlaceholder);
APP_LOCALIZED_CSTRING(lists_browser, kFolderNamePlaceholder);
APP_LOCALIZED_CSTRING(lists_browser, kRenamePlaceholder);
APP_LOCALIZED_CSTRING(lists_browser, kRenameButton);
APP_LOCALIZED_CSTRING(lists_browser, kCreateListPrompt);
APP_LOCALIZED_CSTRING(lists_browser, kCreateFolderPrompt);
APP_LOCALIZED_CSTRING(lists_browser, kRenamePrompt);
APP_LOCALIZED_CSTRING(lists_browser, kDeletePrompt);
APP_LOCALIZED_CSTRING(lists_browser, kNewFolderButton);
APP_LOCALIZED_CSTRING(lists_browser, kNewListButton);
APP_LOCALIZED_CSTRING(lists_browser, kRenameToolbarButton);
APP_LOCALIZED_CSTRING(lists_browser, kDeleteToolbarButton);
APP_LOCALIZED_CSTRING(lists_browser, kLanguageButton);
APP_LOCALIZED_CSTRING(lists_browser, kTitle);

inline std::string currentPath(const std::string& path) {
	return isEnglishLocale() ? en::lists_browser::currentPath(path) : pl::lists_browser::currentPath(path);
}
} // namespace lists_browser

} // namespace runtime

#undef APP_LOCALIZED_CSTRING
#undef APP_LOCALIZED_CHAR

namespace selected = runtime;

}  // namespace app::localization