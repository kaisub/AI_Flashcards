#include "core/StudySession.hpp"

#include <algorithm>
#include <memory>
#include <random>

namespace core {

    StudySession::StudySession(const std::vector<std::shared_ptr<Flashcard>>& deck, const SessionConfig& config, std::optional<unsigned int> seed)
        : currentConfig(config),
          randomEngine(seed.has_value() ? seed.value() : std::random_device{}()) {

        initializeQueues(deck);
    }

    void StudySession::initializeQueues(const std::vector<std::shared_ptr<Flashcard>>& deck) {
        // Clear existing queues if re-initializing
        queueNew.clear();
        queueKnown.clear();
        queueMastered.clear();
        undoManager.clear(); // Clear history on new session/deck

        for (const auto& card : deck) {
            if (!card) { continue; } // Skip nullptrs

            TranslationDirection actualDirection = currentConfig.direction;
            if (actualDirection == TranslationDirection::Mixed) {
                actualDirection = getRandomDirection();
            }

            const ReviewItem item{card, actualDirection};

            const CardState state = (actualDirection == TranslationDirection::Front_to_Back) ?
                               card->state_Front_to_Back : card->state_Back_to_Front;

            switch (state) {
                case CardState::New:
                    queueNew.push_back(item);
                    break;
                case CardState::Known:
                    queueKnown.push_back(item);
                    break;
                case CardState::Mastered:
                    queueMastered.push_back(item);
                    break;
            }
        }

        // Shuffle all queues initially
        std::shuffle(queueNew.begin(), queueNew.end(), randomEngine);
        std::shuffle(queueKnown.begin(), queueKnown.end(), randomEngine);
        std::shuffle(queueMastered.begin(), queueMastered.end(), randomEngine);
    }

    void StudySession::updateConfig(const SessionConfig& newConfig) {
        // This simple update means that future draws will respect the new config.
        // It does not re-evaluate existing queues or re-initialize.
        // If a full re-initialization based on new config is needed (e.g., if focusedTargetState changes
        // dramatically and requires re-population), a separate method or constructor overload might be preferred.
        currentConfig = newConfig;
    }

    size_t StudySession::getQueueSize(CardState state) const {
        switch (state) {
            case CardState::New: return queueNew.size();
            case CardState::Known: return queueKnown.size();
            case CardState::Mastered: return queueMastered.size();
        }
        return 0; // Should not happen
    }

    size_t StudySession::getTotalRemainingInFocusedMode() const {
        if (currentConfig.type == SessionType::Focused && currentConfig.focusedTargetState) {
            return getQueueSize(*currentConfig.focusedTargetState);
        }
        return 0;
    }

    std::optional<ReviewItem> StudySession::getNextItem() {
        if (currentConfig.type == SessionType::Standard) {
            return drawStandard();
        }
        if (currentConfig.type == SessionType::Focused) {
            return drawFocused();
        }
        return std::nullopt; // Unknown session type
    }

    TranslationDirection StudySession::getRandomDirection() {
        std::uniform_int_distribution<> distrib(0, 1);
        return (distrib(randomEngine) == 0) ? TranslationDirection::Front_to_Back : TranslationDirection::Back_to_Front;
    }

    std::optional<ReviewItem> StudySession::drawStandard() {
        std::vector<std::pair<CardState, unsigned int>> weightedQueues;

        if (!queueNew.empty())      { weightedQueues.emplace_back(CardState::New, currentConfig.weightNew); }
        if (!queueKnown.empty())    { weightedQueues.emplace_back(CardState::Known, currentConfig.weightKnown); }
        if (!queueMastered.empty()) { weightedQueues.emplace_back(CardState::Mastered, currentConfig.weightMastered); }

        if (weightedQueues.empty()) {
            return std::nullopt; // No cards left in any queue
        }

        // Create a distribution from the active weights
        std::vector<unsigned int> weights;
        weights.reserve(weightedQueues.size());
        for (const auto& entry : weightedQueues) {
            weights.push_back(entry.second);
        }
        std::discrete_distribution<> distrib(weights.begin(), weights.end());

        // Select a queue based on the distribution
        const CardState chosenState = weightedQueues[static_cast<size_t>(distrib(randomEngine))].first;

        ReviewItem item;
        std::deque<ReviewItem>* targetQueue = nullptr;

        switch (chosenState) {
            case CardState::New: targetQueue = &queueNew; break;
            case CardState::Known: targetQueue = &queueKnown; break;
            case CardState::Mastered: targetQueue = &queueMastered; break;
        }

        if (targetQueue != nullptr && !targetQueue->empty()) {
            item = targetQueue->front();
            targetQueue->pop_front();

            // Store original card states before modification for undo
            HistoryRecord record;
            record.cardId = item.card->id;
            record.cardPtr = item.card; // Store shared_ptr for direct access
            record.previousFrontState = item.card->state_Front_to_Back;
            record.previousBackState = item.card->state_Back_to_Front;
            record.askedDirection = item.askedDirection;
            // The `pushedToQueueState` will be set in submitAnswer

            undoManager.pushRecord(record);
            return item;
        }

        // This case should ideally not be reached due to weightedQueues check,
        // but as a failsafe, we can retry or return nullopt.
        // For simplicity, we'll return nullopt.
        return std::nullopt;
    }

    std::optional<ReviewItem> StudySession::drawFocused() {
        if (!currentConfig.focusedTargetState) {
            return std::nullopt; // Focused mode without a target state is invalid
        }

        std::deque<ReviewItem>* targetQueue = nullptr;
        switch (*currentConfig.focusedTargetState) {
            case CardState::New: targetQueue = &queueNew; break;
            case CardState::Known: targetQueue = &queueKnown; break;
            case CardState::Mastered: targetQueue = &queueMastered; break;
        }

        if (targetQueue != nullptr && !targetQueue->empty()) {
            ReviewItem item = targetQueue->front();
            targetQueue->pop_front();

            // Store original card states before modification for undo
            HistoryRecord record;
            record.cardId = item.card->id;
            record.cardPtr = item.card;
            record.previousFrontState = item.card->state_Front_to_Back;
            record.previousBackState = item.card->state_Back_to_Front;
            record.askedDirection = item.askedDirection;
            // pushedToQueueState set in submitAnswer

            undoManager.pushRecord(record);
            return item;
        }

        return std::nullopt; // No cards left in the focused queue
    }

    void StudySession::submitAnswer(const ReviewItem& item, CardState newState) {
        if (!item.card) { return; }

        // Update card's state based on the direction it was asked in
        if (item.askedDirection == TranslationDirection::Front_to_Back) {
            item.card->state_Front_to_Back = newState;
        } else if (item.askedDirection == TranslationDirection::Back_to_Front) {
            item.card->state_Back_to_Front = newState;
        }
        // Mixed direction means we would typically update both, but for simplicity
        // in a basic TDD, we'll assign newState to the direction that was asked,
        // implying the user is assessing based on that specific translation.
        // A more complex system might average or apply newState to both if Mixed.
        // For now, if Mixed, treat it as Front_to_Back for state update, or choose one consistently.
        // Let's stick to the direction it was *asked in* for the state update for now.

        // If history stack is not empty, update the last record with the state it was pushed to
        undoManager.updateLastRecordState(item.card->id, newState);

        // Re-queue the card
        TranslationDirection reQueueDirection = currentConfig.direction;
        if (reQueueDirection == TranslationDirection::Mixed) {
            reQueueDirection = getRandomDirection(); // Randomize for the next review
        }
        const ReviewItem newItem{item.card, reQueueDirection};

        switch (newState) {
            case CardState::New:
                queueNew.push_back(newItem);
                break;
            case CardState::Known:
                queueKnown.push_back(newItem);
                break;
            case CardState::Mastered:
                queueMastered.push_back(newItem);
                break;
        }
    }

    bool StudySession::removeCardFromAnyQueue(const std::string& cardId) {
        bool removed = false;

        auto remove_if_id = [&](std::deque<ReviewItem>& q) {
            auto it = std::remove_if(q.begin(), q.end(),
                                    [&](const ReviewItem& item) { return item.card && item.card->id == cardId; });
            if (it != q.end()) {
                q.erase(it, q.end());
                return true;
            }
            return false;
        };

        if (remove_if_id(queueNew)) { removed = true; }
        if (remove_if_id(queueKnown)) { removed = true; }
        if (remove_if_id(queueMastered)) { removed = true; }

        return removed;
    }

    bool StudySession::undoLastAction() {
        auto recordOpt = undoManager.popRecord();
        if (!recordOpt.has_value()) {
            return false;
        }
        const HistoryRecord lastRecord = recordOpt.value();

        // The card was previously submitted and pushed to `lastRecord.pushedToQueueState` queue.
        // Find and remove it from there.
        if (lastRecord.cardPtr && removeCardFromAnyQueue(lastRecord.cardId)) {
            // Restore card's state
            lastRecord.cardPtr->state_Front_to_Back = lastRecord.previousFrontState;
            lastRecord.cardPtr->state_Back_to_Front = lastRecord.previousBackState;

            // Put the card back into its original queue where it was drawn from for the last review
            // Based on its previous state (e.g. if it was New and then became Known, undo means putting it back into New queue).
            const CardState originalState = (lastRecord.askedDirection == TranslationDirection::Front_to_Back) ?
                                       lastRecord.previousFrontState : lastRecord.previousBackState;

            // Re-add to the front of the queue to simulate "putting it back" to be reviewed again soon.
            // Or to the back, if we want it to be reviewed later. Front is more common for undo.
            // For now, let's just put it back at the end of the appropriate queue.
            // This is a design decision. For simplicity and to avoid immediately drawing it again, back is safer.
            // However, a true "undo" might mean putting it back where it was, potentially at the front.
            // The prompt says "places it back in its previous queue", which often implies at the end.

            // Restore the card exactly with the direction it was drawn with
            const ReviewItem restoredItem{lastRecord.cardPtr, lastRecord.askedDirection};

            switch (originalState) {
                case CardState::New:
                    queueNew.push_back(restoredItem);
                    break;
                case CardState::Known:
                    queueKnown.push_back(restoredItem);
                    break;
                case CardState::Mastered:
                    queueMastered.push_back(restoredItem);
                    break;
            }
            return true;
        }
        return false;
    }

    bool StudySession::removeCardFromSession(const std::string& cardId) {
        return removeCardFromAnyQueue(cardId);
    }

} // namespace core