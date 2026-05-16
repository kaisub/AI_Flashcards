#pragma once

#include "core/StudySession.hpp"
#include <optional>

namespace app::model {

    namespace StudyConfigUI {
        enum DirectionOption : int {
            FrontToBack = 0,
            BackToFront = 1,
            Mixed = 2,
        };

        enum OrderOption : int {
            Queue = 0,
            Random = 1,
        };

        enum ModeOption : int {
            Standard = 0,
            Focused = 1,
        };

        enum TargetOption : int {
            New = 0,
            Known = 1,
            Mastered = 2,
        };

        constexpr int kTotalWeight = 100;
        
        // Default domain weights
        constexpr int kDefaultWeightNew = 70;
        constexpr int kDefaultWeightKnown = 23;
        constexpr int kDefaultWeightMastered = 7;
    }

    struct StudyConfigViewModel {
        // --- UI State ---
        int directionSelected = StudyConfigUI::Mixed;
        int orderSelected = StudyConfigUI::Random;
        int modeSelected = StudyConfigUI::Standard;
        int targetStateSelected = StudyConfigUI::New;

        // Weight sliders for sampling proportions
        int weightNew = StudyConfigUI::kDefaultWeightNew;
        int weightKnown = StudyConfigUI::kDefaultWeightKnown;
        int weightMastered = StudyConfigUI::kDefaultWeightMastered;

        // Previous values for reactive auto-balancing
        int prevWeightNew = StudyConfigUI::kDefaultWeightNew;
        int prevWeightKnown = StudyConfigUI::kDefaultWeightKnown;
        int prevWeightMastered = StudyConfigUI::kDefaultWeightMastered;

        // --- UI Logic ---
        void balanceWeights() {
            using namespace StudyConfigUI;

            if (weightKnown != prevWeightKnown) {
                if (weightKnown + weightMastered > kTotalWeight) {
                    weightKnown = kTotalWeight - weightMastered;
                }
                weightNew = kTotalWeight - weightKnown - weightMastered;
            } else if (weightMastered != prevWeightMastered) {
                if (weightKnown + weightMastered > kTotalWeight) {
                    weightMastered = kTotalWeight - weightKnown;
                }
                weightNew = kTotalWeight - weightKnown - weightMastered;
            } else if (weightNew != prevWeightNew) {
                if (weightNew + weightMastered > kTotalWeight) {
                    weightMastered = kTotalWeight - weightNew;
                    weightKnown = 0;
                } else {
                    weightKnown = kTotalWeight - weightNew - weightMastered;
                }
            }
            prevWeightNew = weightNew;
            prevWeightKnown = weightKnown;
            prevWeightMastered = weightMastered;
        }

        // --- Domain Mapping ---
        core::TranslationDirection getSelectedDirection() const {
            switch (directionSelected) {
                case StudyConfigUI::FrontToBack: return core::TranslationDirection::Front_to_Back;
                case StudyConfigUI::BackToFront: return core::TranslationDirection::Back_to_Front;
                case StudyConfigUI::Mixed: return core::TranslationDirection::Mixed;
                default: return core::TranslationDirection::Mixed;
            }
        }

        core::SessionType getSelectedMode() const {
            return modeSelected == StudyConfigUI::Standard ? core::SessionType::Standard : core::SessionType::Focused;
        }

        core::SessionOrder getSelectedOrder() const {
            return orderSelected == StudyConfigUI::Queue ? core::SessionOrder::Queue : core::SessionOrder::Random;
        }

        core::CardState getSelectedTargetState() const {
            switch (targetStateSelected) {
                case StudyConfigUI::New: return core::CardState::New;
                case StudyConfigUI::Known: return core::CardState::Known;
                case StudyConfigUI::Mastered: return core::CardState::Mastered;
                default: return core::CardState::New;
            }
        }
    };

} // namespace app::model