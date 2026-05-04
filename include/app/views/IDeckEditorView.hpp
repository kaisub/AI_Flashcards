#pragma once
#include "app/views/IView.hpp"
#include "core/FlashcardList.hpp"
#include <functional>
#include <string>
#include <memory>
#include <vector>

namespace app {
    class IDeckEditorView : public IView {
    public:
        using AddCardHandler = std::function<void(const std::string& front, const std::string& back)>;
        using RemoveCardHandler = std::function<void(const std::string& id)>;
        using UpdateCardHandler = std::function<void(const std::string& cardId, const std::string& newFront, const std::string& newBack)>;
        using DeleteSelectedHandler = std::function<void(const std::vector<std::string>& cardIds)>;
        using MoveSelectedHandler = std::function<void(const std::vector<std::string>& cardIds, int targetListIndex)>;
        using CopySelectedHandler = std::function<void(const std::vector<std::string>& cardIds, int targetListIndex)>;
        using ImportRequestedHandler = std::function<void(const std::string& path, char delim, bool ignoreHeader)>;
        using StartStudyHandler = std::function<void()>;
        using ExitToBrowserHandler = std::function<void()>;

        virtual ~IDeckEditorView() = default;

        virtual void setDeck(std::shared_ptr<core::FlashcardList> deck) = 0;
        virtual void setAvailableLists(const std::vector<std::string>& listNames) = 0;

        void setOnAddCard(AddCardHandler handler) { onAddCard_ = std::move(handler); }
        void setOnRemoveCard(RemoveCardHandler handler) { onRemoveCard_ = std::move(handler); }
        void setOnUpdateCard(UpdateCardHandler handler) { onUpdateCard_ = std::move(handler); }
        void setOnDeleteSelected(DeleteSelectedHandler handler) { onDeleteSelected_ = std::move(handler); }
        void setOnMoveSelected(MoveSelectedHandler handler) { onMoveSelected_ = std::move(handler); }
        void setOnCopySelected(CopySelectedHandler handler) { onCopySelected_ = std::move(handler); }
        void setOnImportRequested(ImportRequestedHandler handler) { onImportRequested_ = std::move(handler); }
        void setOnStartStudy(StartStudyHandler handler) { onStartStudy_ = std::move(handler); }
        void setOnExitToBrowser(ExitToBrowserHandler handler) { onExitToBrowser_ = std::move(handler); }

        bool triggerAddCard(const std::string& front, const std::string& back) {
            if (!onAddCard_) {
                return false;
            }
            onAddCard_(front, back);
            return true;
        }

        bool triggerRemoveCard(const std::string& id) {
            if (!onRemoveCard_) {
                return false;
            }
            onRemoveCard_(id);
            return true;
        }

        bool triggerUpdateCard(const std::string& cardId, const std::string& newFront, const std::string& newBack) {
            if (!onUpdateCard_) {
                return false;
            }
            onUpdateCard_(cardId, newFront, newBack);
            return true;
        }

        bool triggerDeleteSelected(const std::vector<std::string>& cardIds) {
            if (!onDeleteSelected_) {
                return false;
            }
            onDeleteSelected_(cardIds);
            return true;
        }

        bool triggerMoveSelected(const std::vector<std::string>& cardIds, int targetListIndex) {
            if (!onMoveSelected_) {
                return false;
            }
            onMoveSelected_(cardIds, targetListIndex);
            return true;
        }

        bool triggerCopySelected(const std::vector<std::string>& cardIds, int targetListIndex) {
            if (!onCopySelected_) {
                return false;
            }
            onCopySelected_(cardIds, targetListIndex);
            return true;
        }

        bool triggerImportRequested(const std::string& path, char delim, bool ignoreHeader) {
            if (!onImportRequested_) {
                return false;
            }
            onImportRequested_(path, delim, ignoreHeader);
            return true;
        }

        bool triggerStartStudy() {
            if (!onStartStudy_) {
                return false;
            }
            onStartStudy_();
            return true;
        }

        bool triggerExitToBrowser() {
            if (!onExitToBrowser_) {
                return false;
            }
            onExitToBrowser_();
            return true;
        }

    private:
        AddCardHandler onAddCard_;
        RemoveCardHandler onRemoveCard_;
        UpdateCardHandler onUpdateCard_;
        DeleteSelectedHandler onDeleteSelected_;
        MoveSelectedHandler onMoveSelected_;
        CopySelectedHandler onCopySelected_;
        ImportRequestedHandler onImportRequested_;
        StartStudyHandler onStartStudy_;
        ExitToBrowserHandler onExitToBrowser_;
    };
}
