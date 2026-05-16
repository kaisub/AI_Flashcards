#pragma once

#include "Flashcard.hpp"
#include <deque>
#include <vector>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include "UndoManager.hpp"

namespace core {

    struct ReviewItem {
        std::shared_ptr<Flashcard> card;
        TranslationDirection askedDirection;
    };

    enum class SessionType {
        Standard,
        Focused
    };

    enum class SessionOrder {
        Queue,
        Random,
    };

    struct SessionConfig {
        SessionType type{SessionType::Standard};
        std::optional<CardState> focusedTargetState{std::nullopt};
        TranslationDirection direction{TranslationDirection::Mixed};
        SessionOrder order{SessionOrder::Random};
        
        unsigned int weightNew{70};
        unsigned int weightKnown{23};
        unsigned int weightMastered{7};

        void resetToDefaults() {
            type = SessionType::Standard;
            focusedTargetState = std::nullopt;
            direction = TranslationDirection::Mixed;
            order = SessionOrder::Random;
            weightNew = 70;
            weightKnown = 23;
            weightMastered = 7;
        }
    };

    class StudySession {
    public:
        StudySession(const std::vector<std::shared_ptr<Flashcard>>& deck, const SessionConfig& config, std::optional<unsigned int> seed = std::nullopt);
        
        void updateConfig(const SessionConfig& newConfig);
        size_t getQueueSize(CardState state) const;
        size_t getTotalRemainingInFocusedMode() const;

        // Core flow
        std::optional<ReviewItem> getNextItem();
        void submitAnswer(const ReviewItem& item, CardState newState);
        
        // UX requirements
        bool undoLastAction(); 
        bool removeCardFromSession(const std::string& cardId);

    private:
        struct ShuffleCycleState {
            size_t seenSinceShuffle{0};
            size_t cycleSize{0};
        };

        struct QueueBucket {
            std::deque<ReviewItem> queue;
            ShuffleCycleState shuffle;
        };

        QueueBucket bucketNew;
        QueueBucket bucketKnown;
        QueueBucket bucketMastered;
        
        UndoManager undoManager;
        SessionConfig currentConfig;
        std::mt19937 randomEngine;

        void initializeQueues(const std::vector<std::shared_ptr<Flashcard>>& deck);
        TranslationDirection getRandomDirection();
        bool removeCardFromAnyQueue(const std::string& cardId);
        void resetShuffleCycles();
        void maybeShuffleQueueAfterPass(CardState state);
        QueueBucket& bucketForState(CardState state);
        ShuffleCycleState& cycleStateFor(CardState state);
        std::deque<ReviewItem>& queueForState(CardState state);
        static CardState stateFromAskedDirection(const ReviewItem& item);
        
        std::optional<ReviewItem> drawStandard();
        std::optional<ReviewItem> drawFocused();
    };

} // namespace core