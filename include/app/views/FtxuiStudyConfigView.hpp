#pragma once

#include "IStudyConfigView.hpp"
#include "core/StudySession.hpp"
#include "app/model/StudyConfigViewModel.hpp"

namespace app {
    /**
     * @brief FTXUI implementation of the study configuration view.
     * Allows the user to select session type, direction, and target state before studying.
     */
    class FtxuiStudyConfigView : public IStudyConfigView {
    public:
        FtxuiStudyConfigView() = default;
        ~FtxuiStudyConfigView() override = default;

        // IView override
        void run() override;

    private:
        app::model::StudyConfigViewModel _vm;
    };
}
