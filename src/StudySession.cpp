#include "core/StudySession.hpp"

#include <algorithm>
#include <memory>
#include <random>

namespace core {

    namespace {

    std::optional<ReviewItem> popItemFromQueue(std::deque<ReviewItem>* targetQueue) {
        if (targetQueue == nullptr || targetQueue->empty()) {
            return std::nullopt;
        }

        ReviewItem item = targetQueue->front();
        targetQueue->pop_front();
        return item;
    }

    } // namespace

    StudySession::StudySession(const std::vector<std::shared_ptr<Flashcard>>& deck, const SessionConfig& config, std::optional<unsigned int> seed)
        : currentConfig(config),
          randomEngine(seed.has_value() ? seed.value() : std::random_device{}()) {

        initializeQueues(deck);
    }

    void StudySession::initializeQueues(const std::vector<std::shared_ptr<Flashcard>>& deck) {
        // Clear existing queues if re-initializing
        bucketNew.queue.clear();
        bucketKnown.queue.clear();
        bucketMastered.queue.clear();
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
                    bucketNew.queue.push_back(item);
                    break;
                case CardState::Known:
                    bucketKnown.queue.push_back(item);
                    break;
                case CardState::Mastered:
                    bucketMastered.queue.push_back(item);
                    break;
            }
        }

        if (currentConfig.order == SessionOrder::Random) {
            std::shuffle(bucketNew.queue.begin(), bucketNew.queue.end(), randomEngine);
            std::shuffle(bucketKnown.queue.begin(), bucketKnown.queue.end(), randomEngine);
            std::shuffle(bucketMastered.queue.begin(), bucketMastered.queue.end(), randomEngine);
        }
        resetShuffleCycles();
    }

    void StudySession::updateConfig(const SessionConfig& newConfig) {
        // This simple update means that future draws will respect the new config.
        // It does not re-evaluate existing queues or re-initialize.
        // If a full re-initialization based on new config is needed (e.g., if focusedTargetState changes
        // dramatically and requires re-population), a separate method or constructor overload might be preferred.
        const bool wasRandomOrder = (currentConfig.order == SessionOrder::Random);
        currentConfig = newConfig;

        if (!wasRandomOrder && currentConfig.order == SessionOrder::Random) {
            std::shuffle(bucketNew.queue.begin(), bucketNew.queue.end(), randomEngine);
            std::shuffle(bucketKnown.queue.begin(), bucketKnown.queue.end(), randomEngine);
            std::shuffle(bucketMastered.queue.begin(), bucketMastered.queue.end(), randomEngine);
        }
        resetShuffleCycles();
    }

    void StudySession::resetShuffleCycles() {
        bucketNew.shuffle = ShuffleCycleState{0, bucketNew.queue.size()};
        bucketKnown.shuffle = ShuffleCycleState{0, bucketKnown.queue.size()};
        bucketMastered.shuffle = ShuffleCycleState{0, bucketMastered.queue.size()};
    }

    StudySession::QueueBucket& StudySession::bucketForState(CardState state) {
        switch (state) {
            case CardState::New:
                return bucketNew;
            case CardState::Known:
                return bucketKnown;
            case CardState::Mastered:
                return bucketMastered;
        }
        return bucketNew;
    }

    StudySession::ShuffleCycleState& StudySession::cycleStateFor(CardState state) {
        return bucketForState(state).shuffle;
    }

    std::deque<ReviewItem>& StudySession::queueForState(CardState state) {
        return bucketForState(state).queue;
    }

    CardState StudySession::stateFromAskedDirection(const ReviewItem& item) {
        if (item.askedDirection == TranslationDirection::Back_to_Front) {
            return item.card->state_Back_to_Front;
        }
        return item.card->state_Front_to_Back;
    }

    void StudySession::maybeShuffleQueueAfterPass(CardState state) {
        if (currentConfig.order != SessionOrder::Random) {
            return;
        }

        auto& queue = queueForState(state);
        auto& cycleState = cycleStateFor(state);

        if (cycleState.cycleSize == 0) {
            cycleState.cycleSize = queue.size();
        }

        if (cycleState.cycleSize > 0 && cycleState.seenSinceShuffle >= cycleState.cycleSize) {
            if (queue.size() > 1) {
                std::shuffle(queue.begin(), queue.end(), randomEngine);
            }
            cycleState.seenSinceShuffle = 0;
            cycleState.cycleSize = queue.size();
        }
    }

    size_t StudySession::getQueueSize(CardState state) const {
        switch (state) {
            case CardState::New: return bucketNew.queue.size();
            case CardState::Known: return bucketKnown.queue.size();
            case CardState::Mastered: return bucketMastered.queue.size();
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

        if (!bucketNew.queue.empty())      { weightedQueues.emplace_back(CardState::New, currentConfig.weightNew); }
        if (!bucketKnown.queue.empty())    { weightedQueues.emplace_back(CardState::Known, currentConfig.weightKnown); }
        if (!bucketMastered.queue.empty()) { weightedQueues.emplace_back(CardState::Mastered, currentConfig.weightMastered); }

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

        std::deque<ReviewItem>* targetQueue = nullptr;

        switch (chosenState) {
            case CardState::New: targetQueue = &bucketNew.queue; break;
            case CardState::Known: targetQueue = &bucketKnown.queue; break;
            case CardState::Mastered: targetQueue = &bucketMastered.queue; break;
        }

        if (const auto nextItem = popItemFromQueue(targetQueue); nextItem.has_value()) {
            if (currentConfig.order == SessionOrder::Random) {
                ++cycleStateFor(chosenState).seenSinceShuffle;
            }
            return nextItem;
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
            case CardState::New: targetQueue = &bucketNew.queue; break;
            case CardState::Known: targetQueue = &bucketKnown.queue; break;
            case CardState::Mastered: targetQueue = &bucketMastered.queue; break;
        }

        if (const auto nextItem = popItemFromQueue(targetQueue); nextItem.has_value()) {
            if (currentConfig.order == SessionOrder::Random) {
                ++cycleStateFor(*currentConfig.focusedTargetState).seenSinceShuffle;
            }
            return nextItem;
        }

        return std::nullopt; // No cards left in the focused queue
    }

    void StudySession::submitAnswer(const ReviewItem& item, CardState newState) {
        if (!item.card) { return; }

        const CardState sourceState = stateFromAskedDirection(item);

        // Undo should revert the last submitted answer, so record history here.
        HistoryRecord record;
        record.cardId = item.card->card_id;
        record.cardPtr = item.card;
        record.previousFrontState = item.card->state_Front_to_Back;
        record.previousBackState = item.card->state_Back_to_Front;
        record.askedDirection = item.askedDirection;
        record.pushedToQueueState = newState;
        undoManager.pushRecord(record);

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

        // Re-queue the card
        TranslationDirection reQueueDirection = currentConfig.direction;
        if (reQueueDirection == TranslationDirection::Mixed) {
            reQueueDirection = getRandomDirection(); // Randomize for the next review
        }
        const ReviewItem newItem{item.card, reQueueDirection};

        switch (newState) {
            case CardState::New:
                bucketNew.queue.push_back(newItem);
                break;
            case CardState::Known:
                bucketKnown.queue.push_back(newItem);
                break;
            case CardState::Mastered:
                bucketMastered.queue.push_back(newItem);
                break;
        }

        if (currentConfig.order == SessionOrder::Random) {
            maybeShuffleQueueAfterPass(sourceState);

            auto& destinationCycle = cycleStateFor(newState);
            if (destinationCycle.cycleSize == 0) {
                destinationCycle.cycleSize = queueForState(newState).size();
            }
        }
    }

    bool StudySession::removeCardFromAnyQueue(const std::string& cardId) {
        bool removed = false;

        auto remove_if_id = [&](std::deque<ReviewItem>& que) {
            auto itr = std::remove_if(que.begin(), que.end(),
                                    [&](const ReviewItem& item) { return item.card && item.card->card_id == cardId; });
            if (itr != que.end()) {
                que.erase(itr, que.end());
                return true;
            }
            return false;
        };

        if (remove_if_id(bucketNew.queue)) { removed = true; }
        if (remove_if_id(bucketKnown.queue)) { removed = true; }
        if (remove_if_id(bucketMastered.queue)) { removed = true; }

        return removed;
    }

    bool StudySession::undoLastAction() {
        auto recordOpt = undoManager.popRecord();
        if (!recordOpt.has_value()) {
            return false;
        }
        const HistoryRecord lastRecord = recordOpt.value();

        if (!lastRecord.cardPtr) {
            return false;
        }

        auto remove_last_matching = [&](std::deque<ReviewItem>& que, const std::string& cardId) {
            for (auto it = que.end(); it != que.begin();) {
                --it;
                if (it->card && it->card->card_id == cardId) {
                    que.erase(it);
                    return true;
                }
            }
            return false;
        };

        std::deque<ReviewItem>* submittedQueue = nullptr;
        switch (lastRecord.pushedToQueueState) {
            case CardState::New:
                submittedQueue = &bucketNew.queue;
                break;
            case CardState::Known:
                submittedQueue = &bucketKnown.queue;
                break;
            case CardState::Mastered:
                submittedQueue = &bucketMastered.queue;
                break;
        }

        bool removedFromSubmittedQueue = false;
        if (submittedQueue != nullptr) {
            removedFromSubmittedQueue = remove_last_matching(*submittedQueue, lastRecord.cardId);
        }
        if (!removedFromSubmittedQueue) {
            // Fallback: keep previous behavior if queue metadata drifted.
            removedFromSubmittedQueue = removeCardFromAnyQueue(lastRecord.cardId);
        }

        // The card was previously submitted and pushed to `lastRecord.pushedToQueueState` queue.
        // Find and remove it from there.
        if (removedFromSubmittedQueue) {
            // Restore card's state
            lastRecord.cardPtr->state_Front_to_Back = lastRecord.previousFrontState;
            lastRecord.cardPtr->state_Back_to_Front = lastRecord.previousBackState;

            // Put the card back into its original queue where it was drawn from for the last review
            // Based on its previous state (e.g. if it was New and then became Known, undo means putting it back into New queue).
            const CardState originalState = (lastRecord.askedDirection == TranslationDirection::Front_to_Back) ?
                                       lastRecord.previousFrontState : lastRecord.previousBackState;

            // Put undone card at the front so "undo" immediately restores the previous review flow.

            // Restore the card exactly with the direction it was drawn with
            const ReviewItem restoredItem{lastRecord.cardPtr, lastRecord.askedDirection};

            switch (originalState) {
                case CardState::New:
                    bucketNew.queue.push_front(restoredItem);
                    break;
                case CardState::Known:
                    bucketKnown.queue.push_front(restoredItem);
                    break;
                case CardState::Mastered:
                    bucketMastered.queue.push_front(restoredItem);
                    break;
            }
            if (currentConfig.order == SessionOrder::Random) {
                resetShuffleCycles();
            }
            return true;
        }
        return false;
    }

    bool StudySession::removeCardFromSession(const std::string& cardId) {
        const bool removed = removeCardFromAnyQueue(cardId);
        if (removed && currentConfig.order == SessionOrder::Random) {
            resetShuffleCycles();
        }
        return removed;
    }

} // namespace core