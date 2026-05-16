#pragma once

#include "Flashcard.hpp"
#include <vector>
#include <string>
#include <memory>
#include <optional>

namespace core {

    struct HistoryRecord {
        std::string cardId;
        std::shared_ptr<Flashcard> cardPtr;
        CardState previousFrontState{};
        CardState previousBackState{};
        TranslationDirection askedDirection{};
        CardState pushedToQueueState{};
    };

    class UndoManager {
    public:
        void pushRecord(const HistoryRecord& record) {
            historyStack.push_back(record);
        }

        std::optional<HistoryRecord> popRecord() {
            if (historyStack.empty()) {
                return std::nullopt;
            }
            HistoryRecord record = historyStack.back();
            historyStack.pop_back();
            return record;
        }

        void clear() {
            historyStack.clear();
        }

        bool isEmpty() const {
            return historyStack.empty();
        }

    private:
        std::vector<HistoryRecord> historyStack;
    };

} // namespace core