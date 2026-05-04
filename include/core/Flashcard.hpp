#pragma once

#include <string>

namespace core {

    enum class CardState {
        New,
        Known,
        Mastered
    };

    enum class TranslationDirection {
        Front_to_Back,
        Back_to_Front,
        Mixed
    };

    struct Flashcard {
        std::string id;
        std::string text_front;
        std::string text_back;
        
        CardState state_Front_to_Back{CardState::New};
        CardState state_Back_to_Front{CardState::New};

        bool operator==(const Flashcard&) const = default;

        static Flashcard makeCloneAsNew(const Flashcard& source, const std::string& newId) {
            Flashcard cloned;
            cloned.id = newId;
            cloned.text_front = source.text_front;
            cloned.text_back = source.text_back;
            cloned.state_Front_to_Back = CardState::New;
            cloned.state_Back_to_Front = CardState::New;
            return cloned;
        }
    };

} // namespace core