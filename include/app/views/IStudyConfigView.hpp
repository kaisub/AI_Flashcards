#pragma once

#include "IView.hpp"
#include "core/StudySession.hpp"
#include <functional>

namespace app {
    /**
     * @brief Interface for the study configuration view.
     * This view allows the user to configure session parameters before starting a study session.
     */
    class IStudyConfigView : public IView {
    public:
        using StartHandler = std::function<void(core::SessionConfig)>;
        using CancelHandler = std::function<void()>;

        virtual ~IStudyConfigView() = default;

        void setOnStart(StartHandler handler) { onStart_ = std::move(handler); }
        void setOnCancel(CancelHandler handler) { onCancel_ = std::move(handler); }

        bool triggerStart(core::SessionConfig config) {
            if (!onStart_) {
                return false;
            }
            onStart_(std::move(config));
            return true;
        }

        bool triggerCancel() {
            if (!onCancel_) {
                return false;
            }
            onCancel_();
            return true;
        }

    private:
        StartHandler onStart_;
        CancelHandler onCancel_;
    };
}
