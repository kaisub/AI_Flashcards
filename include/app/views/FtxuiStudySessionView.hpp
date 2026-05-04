#pragma once
#include "app/views/IStudySessionView.hpp"
#include "core/Flashcard.hpp"
#include "app/model/StudySessionViewModel.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <string>
#include <vector>

namespace app {
    class FtxuiStudySessionView : public IStudySessionView {
    public:
        FtxuiStudySessionView() = default;
        ~FtxuiStudySessionView() override = default;

        // IView override
        void run() override;

        // IStudySessionView overrides
        void showCard(const core::Flashcard& card, bool showBack) override;
        void showSessionComplete() override;
        void setAvailableLists(const std::vector<std::string>& listNames) override;

    private:
        app::model::StudySessionViewModel _vm;
        
        bool _isEditing = false;
        
        bool _isCopying = false;
        std::vector<std::string> _availableLists;
        int _selectedListIndex = 0;
        
        bool _isDeleting = false;
        
        // Modal / view builders
        ftxui::Component buildSessionCompleteView(ftxui::ScreenInteractive& screen, bool& returnToController);
        ftxui::Component buildDeleteModal(ftxui::ScreenInteractive& screen, bool& returnToController);
        ftxui::Component buildEditModal(ftxui::ScreenInteractive& screen);
        ftxui::Component buildCopyModal(ftxui::ScreenInteractive& screen);
        ftxui::Component buildCardView(ftxui::ScreenInteractive& screen, bool& returnToController, int& focusedButtonIndex);
    };
} // namespace app
