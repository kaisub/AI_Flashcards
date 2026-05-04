#include "app/presenters/StudyConfigPresenter.hpp"

namespace app::presenters {

    StudyConfigPresenter::StudyConfigPresenter(std::shared_ptr<IStudyConfigView> view)
        : _view(std::move(view)) {
        bindEvents();
    }

    void StudyConfigPresenter::start() {
        _view->run();
    }

    void StudyConfigPresenter::bindEvents() {
        _view->setOnStart([this](core::SessionConfig config) {
            if (onStart) {
                onStart(config);
            }
        });
        _view->setOnCancel([this]() {
            if (onCancel) {
                onCancel();
            }
        });
    }
} // namespace app::presenters