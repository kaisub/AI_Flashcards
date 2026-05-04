#pragma once

#include "app/localization/English.hpp"
#include "app/localization/Polish.hpp"

namespace app::localization {

#if defined(AI_FLASHCARDS_LOCALE_PL)
namespace selected = pl;
#elif defined(AI_FLASHCARDS_LOCALE_EN)
namespace selected = en;
#else
#error "No UI locale selected. Define AI_FLASHCARDS_LOCALE_PL or AI_FLASHCARDS_LOCALE_EN."
#endif

}  // namespace app::localization