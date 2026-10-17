#ifndef EMESSAGEHISTORY_H
#define EMESSAGEHISTORY_H

#include <cstddef>
#include <vector>

#include "emessage.h"
#include "characters/echaracter.h"
#include "engine/eeventdata.h"

struct eSavedMessage {
    eEventData fEd;
    eMessage fMsg;
    // Original short notification text; the inbox always keeps fMsg (full).
    eMessage fCondensed;
};

struct eMessageSidebarState {
    std::size_t fCount = 0;
    const eSavedMessage* fLatest = nullptr;
};

std::string eMessageSubject(const eSavedMessage& saved);

class eMessageHistory {
public:
    void add(const eEventData& ed, const eMessage& msg,
             const eMessage& condensed = {}) {
        auto& saved = mMessages.emplace_back();
        saved.fEd = ed;
        saved.fMsg = msg;
        saved.fCondensed = condensed;

        saved.fEd.fType = eMessageEventType::common;
        saved.fEd.fTile = nullptr;
        saved.fEd.fChar.clear();
        saved.fEd.fCA0 = nullptr;
        saved.fEd.fA0 = nullptr;
        saved.fEd.fA1 = nullptr;
        saved.fEd.fA2 = nullptr;
        saved.fEd.fCCA0.clear();
    }

    bool empty() const { return mMessages.empty(); }
    std::size_t size() const { return mMessages.size(); }

    const eSavedMessage* newest(const std::size_t index) const {
        if(index >= mMessages.size()) return nullptr;
        return &mMessages[mMessages.size() - index - 1];
    }

    eMessageSidebarState sidebarState() const {
        return {mMessages.size(), newest(0)};
    }

    void removeNewest(const std::size_t index) {
        if(index >= mMessages.size()) return;
        mMessages.erase(mMessages.end() - index - 1);
    }
private:
    std::vector<eSavedMessage> mMessages;
};

#endif // EMESSAGEHISTORY_H
