#include <gtest/gtest.h>
#include "app/model/StudyConfigViewModel.hpp"

using namespace app::model;
using namespace core;

TEST(StudyConfigViewModelTest, InitialState) {
    StudyConfigViewModel vm;
    EXPECT_EQ(vm.getSelectedDirection(), TranslationDirection::Mixed);
    EXPECT_EQ(vm.getSelectedMode(), SessionType::Standard);
    EXPECT_EQ(vm.getSelectedTargetState(), CardState::New);
    EXPECT_EQ(vm.weightNew, 70);
    EXPECT_EQ(vm.weightKnown, 23);
    EXPECT_EQ(vm.weightMastered, 7);
}

TEST(StudyConfigViewModelTest, BalanceWeights_AdjustsNewWhenKnownChanges) {
    StudyConfigViewModel vm;
    vm.weightKnown = 50; // User increases known
    vm.balanceWeights();
    EXPECT_EQ(vm.weightKnown, 50);
    EXPECT_EQ(vm.weightMastered, 7); // Remains untouched
    EXPECT_EQ(vm.weightNew, 43); // Adjusted to balance (100 - 50 - 7)
}

TEST(StudyConfigViewModelTest, BalanceWeights_AdjustsMasteredWhenNewChanges) {
    StudyConfigViewModel vm;
    vm.weightNew = 100; // User slides 'New' all the way to 100
    vm.balanceWeights();
    EXPECT_EQ(vm.weightNew, 100);
    EXPECT_EQ(vm.weightKnown, 0); // Forced to 0
    EXPECT_EQ(vm.weightMastered, 0); // Forced to 0
}

TEST(StudyConfigViewModelTest, BalanceWeights_CapsAt100) {
    StudyConfigViewModel vm;
    vm.weightKnown = 100; // User slides 'Known' to 100
    vm.balanceWeights();
    
    // Since 'Mastered' was 7 and wasn't the target being dragged, Known is capped
    EXPECT_EQ(vm.weightKnown, 93); 
    EXPECT_EQ(vm.weightMastered, 7);
    EXPECT_EQ(vm.weightNew, 0); // New absorbs the remaining deficit
}

TEST(StudyConfigViewModelTest, GettersMapCorrectly) {
    StudyConfigViewModel vm;
    
    // Test Directions
    vm.directionSelected = 0;
    EXPECT_EQ(vm.getSelectedDirection(), TranslationDirection::Front_to_Back);
    vm.directionSelected = 1;
    EXPECT_EQ(vm.getSelectedDirection(), TranslationDirection::Back_to_Front);
    
    // Test Modes
    vm.modeSelected = 1;
    EXPECT_EQ(vm.getSelectedMode(), SessionType::Focused);
    vm.modeSelected = 0;
    EXPECT_EQ(vm.getSelectedMode(), SessionType::Standard);
    
    // Test Targets
    vm.targetStateSelected = 2;
    EXPECT_EQ(vm.getSelectedTargetState(), CardState::Mastered);
}