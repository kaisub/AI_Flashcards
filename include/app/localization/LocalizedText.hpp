#pragma once

#include <ftxui/util/ref.hpp>
#include <string>

namespace app::localization {

bool isEnglishLocale();

struct LocalizedChar {
	char english;
	char polish;

	operator char() const {
		return isEnglishLocale() ? english : polish;
	}
};

struct LocalizedCString {
	const char* english;
	const char* polish;

	operator std::string() const {
		return isEnglishLocale() ? english : polish;
	}

	operator ftxui::ConstStringRef() const {
		return ftxui::ConstStringRef(isEnglishLocale() ? english : polish);
	}

	operator ftxui::StringRef() const {
		return ftxui::StringRef(isEnglishLocale() ? english : polish);
	}

	std::string str() const {
		return isEnglishLocale() ? english : polish;
	}

	bool starts_with(const std::string& prefix) const {
		return str().starts_with(prefix);
	}

	bool starts_with(const char* prefix) const {
		return str().starts_with(prefix);
	}
};

}  // namespace app::localization