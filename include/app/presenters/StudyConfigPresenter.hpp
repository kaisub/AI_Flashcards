#pragma once

#include "app/views/IStudyConfigView.hpp"
#include "core/StudySession.hpp"
#include <functional>
#include <memory>

namespace app::presenters {

    class StudyConfigPresenter {
    public:
        explicit StudyConfigPresenter(std::shared_ptr<IStudyConfigView> view);

        void start();

        std::function<void(core::SessionConfig)> onStart;
        std::function<void()> onCancel;

    private:
        std::shared_ptr<IStudyConfigView> _view;
        void bindEvents();
    };

} // namespace app::presenters