#pragma once

#include "app/localization/LocalizationData.hpp"

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

namespace selected = text;

}  // namespace app::localization