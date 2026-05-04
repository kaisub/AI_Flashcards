#pragma once

#include "core/Flashcard.hpp"
#include <string>

namespace app::model {

    enum class CheckResult { None, Correct, Incorrect };

    /**
     * @brief Holds the presentation state for the active study session card.
     * This is entirely UI-agnostic and can be shared across any UI framework.
     */
    struct StudySessionViewModel {
        // Core presentation state
        std::string currentFront;
        std::string currentBack;
        bool isFlipped = false;
        bool isComplete = false;
        core::CardState currentCardState = core::CardState::New;
        
        // In-flight editing buffers
        std::string editFront;
        std::string editBack;

        // Answer checker state
        std::string userInput;
        CheckResult lastCheckResult = CheckResult::None;

        void setCard(const core::Flashcard& card, bool showBack) {
            currentFront = card.text_front;
            currentBack = card.text_back;
            isFlipped = showBack;
            isComplete = false;
            currentCardState = card.state_Front_to_Back;
            editFront = card.text_front;
            editBack = card.text_back;
            userInput.clear();
            lastCheckResult = CheckResult::None;
        }
    };

} // namespace app::model