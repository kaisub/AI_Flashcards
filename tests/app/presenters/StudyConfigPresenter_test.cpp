#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "app/presenters/StudyConfigPresenter.hpp"
#include "app/views/IStudyConfigView.hpp"
#include "core/StudySession.hpp"

namespace app {

class MockIStudyConfigView : public IStudyConfigView {
public:
    MOCK_METHOD(void, run, (), (override));
};

class StudyConfigPresenterTest : public ::testing::Test {
protected:
    std::shared_ptr<MockIStudyConfigView> mockView;
    std::unique_ptr<presenters::StudyConfigPresenter> presenter;

    void SetUp() override {
        mockView = std::make_shared<MockIStudyConfigView>();
        presenter = std::make_unique<presenters::StudyConfigPresenter>(mockView);
    }
};

TEST_F(StudyConfigPresenterTest, OnStartFiresCallback) {
    bool wasCalled = false;
    core::SessionConfig receivedConfig;

    presenter->onStart = [&](core::SessionConfig config) {
        wasCalled = true;
        receivedConfig = config;
    };

    core::SessionConfig testConfig;
    testConfig.type = core::SessionType::Focused;
    ASSERT_TRUE(mockView->triggerStart(testConfig));

    EXPECT_TRUE(wasCalled);
    EXPECT_EQ(receivedConfig.type, core::SessionType::Focused);
}

TEST_F(StudyConfigPresenterTest, OnCancelFiresCallback) {
    bool wasCalled = false;
    presenter->onCancel = [&]() { wasCalled = true; };
    ASSERT_TRUE(mockView->triggerCancel());
    EXPECT_TRUE(wasCalled);
}
} // namespace app