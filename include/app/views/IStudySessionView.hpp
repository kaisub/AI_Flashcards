#pragma once
#include "app/views/IView.hpp"
#include "core/Flashcard.hpp"
#include <functional>
#include <vector>
#include <string>

namespace app {
    class IStudySessionView : public IView {
    public:
        using CardRatedHandler = std::function<void(core::CardState newState)>;
        using ExitRequestedHandler = std::function<void()>;
        using UndoRequestedHandler = std::function<void()>;
        using ResetAllRequestedHandler = std::function<void()>;
        using EditRequestedHandler = std::function<void(const std::string& newFront, const std::string& newBack)>;
        using DeleteRequestedHandler = std::function<void()>;
        using CopyRequestedHandler = std::function<void(int selectedListIndex)>;

        virtual ~IStudySessionView() = default;

        // --- Controller -> View ---
        virtual void showCard(const core::Flashcard& card, bool showBack) = 0;
        virtual void showSessionComplete() = 0;
        virtual void setAvailableLists(const std::vector<std::string>& listNames) = 0;

        void setOnCardRated(CardRatedHandler handler) { onCardRated_ = std::move(handler); }
        void setOnExitRequested(ExitRequestedHandler handler) { onExitRequested_ = std::move(handler); }
        void setOnUndoRequested(UndoRequestedHandler handler) { onUndoRequested_ = std::move(handler); }
        void setOnResetAllRequested(ResetAllRequestedHandler handler) { onResetAllRequested_ = std::move(handler); }
        void setOnEditRequested(EditRequestedHandler handler) { onEditRequested_ = std::move(handler); }
        void setOnDeleteRequested(DeleteRequestedHandler handler) { onDeleteRequested_ = std::move(handler); }
        void setOnCopyRequested(CopyRequestedHandler handler) { onCopyRequested_ = std::move(handler); }

        bool triggerCardRated(core::CardState newState) {
            if (!onCardRated_) {
                return false;
            }
            onCardRated_(newState);
            return true;
        }

        bool triggerExitRequested() {
            if (!onExitRequested_) {
                return false;
            }
            onExitRequested_();
            return true;
        }

        bool triggerUndoRequested() {
            if (!onUndoRequested_) {
                return false;
            }
            onUndoRequested_();
            return true;
        }

        bool triggerResetAllRequested() {
            if (!onResetAllRequested_) {
                return false;
            }
            onResetAllRequested_();
            return true;
        }

        bool triggerEditRequested(const std::string& newFront, const std::string& newBack) {
            if (!onEditRequested_) {
                return false;
            }
            onEditRequested_(newFront, newBack);
            return true;
        }

        bool triggerDeleteRequested() {
            if (!onDeleteRequested_) {
                return false;
            }
            onDeleteRequested_();
            return true;
        }

        bool triggerCopyRequested(int selectedListIndex) {
            if (!onCopyRequested_) {
                return false;
            }
            onCopyRequested_(selectedListIndex);
            return true;
        }

    private:
        CardRatedHandler onCardRated_;
        ExitRequestedHandler onExitRequested_;
        UndoRequestedHandler onUndoRequested_;
        ResetAllRequestedHandler onResetAllRequested_;
        EditRequestedHandler onEditRequested_;
        DeleteRequestedHandler onDeleteRequested_;
        CopyRequestedHandler onCopyRequested_;
    };
}
