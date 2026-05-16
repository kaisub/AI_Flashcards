#pragma once

#include "app/views/IDeckEditorView.hpp"
#include "app/model/DeckEditorViewModel.hpp"
#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <memory>
#include <map>
#include <string>
#include <vector>

namespace app {

class FtxuiDeckEditorView : public IDeckEditorView {
public:
    FtxuiDeckEditorView() = default;
    ~FtxuiDeckEditorView() override = default;

    void setDeck(std::shared_ptr<core::FlashcardList> deck) override;
    void setAvailableLists(const std::vector<std::string>& listNames) override;
    void run() override;

private:
    app::model::DeckEditorViewModel _vm;
    std::function<void()> _onDeckChangedInternal;

    // Modal states
    bool _isEditing = false;
    bool _isDeletingBulk = false;
    bool _isMovingBulk = false;
    bool _isCopyingBulk = false;
    bool _isImporting = false;

    // Focus tracking
    std::string _focusedCardId = "";

    // Import modal state
    std::string _importPath = "";
    int _importDelimIndex = 1; // Default to ';'
    std::string _importCustomDelim = ";";
    bool _importIgnoreHeader = true;
    std::vector<std::string> _importDelimiterOptions;

    // File Picker state
    bool _isFilePickerActive = false;
    std::filesystem::path _pickerCurrentPath;
    std::vector<std::filesystem::path> _pickerFullPaths;
    std::vector<std::string> _pickerMenuEntries;
    int _pickerSelectedIndex = 0;

    // Keep edit modal component persistent while open.
    ftxui::Component _editModal;

    void refreshFilePicker();
    ftxui::Component buildFilePickerModal(const ftxui::ButtonOption& btnStyle);

    // Modal builders
    ftxui::Component buildEditModal(const ftxui::ButtonOption& btnStyle);
    ftxui::Component buildDeleteModal(const ftxui::ButtonOption& btnStyle);
    ftxui::Component buildMoveModal(const ftxui::ButtonOption& btnStyle);
    ftxui::Component buildCopyModal(const ftxui::ButtonOption& btnStyle);
    ftxui::Component buildImportModal(const ftxui::ButtonOption& btnStyle, const ftxui::ButtonOption& inlineBtnStyle);
};

} // namespace app
