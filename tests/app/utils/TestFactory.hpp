#pragma once

#include "core/Flashcard.hpp"
#include <memory>
#include <string>

namespace test_utils {

    inline std::shared_ptr<core::Flashcard> createTestFlashcard(
        const std::string& id,
        const std::string& front,
        const std::string& back,
        core::CardState f2b = core::CardState::New,
        core::CardState b2f = core::CardState::New)
    {
        auto card = std::make_shared<core::Flashcard>();
        card->card_id = id;
        card->text_front = front;
        card->text_back = back;
        card->state_Front_to_Back = f2b;
        card->state_Back_to_Front = b2f;
        return card;
    }

} // namespace test_utils