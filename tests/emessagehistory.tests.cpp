#define SDL_MAIN_HANDLED

#include "widgets/emessagehistory.h"

eDate::eDate(const int day, const eMonth month, const int year) :
    mDay(day), mMonth(month), mYear(year) {}

namespace {
bool testEmptyHistory() {
    const eMessageHistory history;
    return history.empty() && history.size() == 0 &&
           history.newest(0) == nullptr;
}

bool testMessagesAreNewestFirst() {
    eMessageHistory history;
    eEventData firstData;
    firstData.fDate = eDate(1, eMonth::january, -1000);
    eEventData secondData;
    secondData.fDate = eDate(2, eMonth::february, -1000);

    history.add(firstData, {"First", "First body"});
    history.add(secondData, {"Second", "Second body"});

    const auto first = history.newest(0);
    const auto second = history.newest(1);
    return history.size() == 2 && first && second &&
           first->fMsg.fTitle == "Second" &&
           second->fMsg.fTitle == "First";
}

bool testStoredMessagesCannotRetainLiveActionsOrLocations() {
    eMessageHistory history;
    eEventData data;
    data.fTile = reinterpret_cast<eTile*>(1);
    data.fType = eMessageEventType::generalRequestGranted;
    data.fA0 = []() {};
    data.fA1 = []() {};
    data.fA2 = []() {};
    data.fCA0 = [](const eAction&) {};
    data.fCCA0[eCityId::neutralFriendly] = []() {};

    history.add(data, {"Safe", "Safe body"});
    const auto saved = history.newest(0);
    return saved && saved->fEd.fTile == nullptr &&
           saved->fEd.fType == eMessageEventType::common &&
           saved->fEd.fChar.isNull() && !saved->fEd.fA0 &&
           !saved->fEd.fA1 && !saved->fEd.fA2 &&
           !saved->fEd.fCA0 && saved->fEd.fCCA0.empty();
}

bool testCondensedPreviewDoesNotReplaceFullMessage() {
    eMessageHistory history;
    history.add({}, {"Full title", "Full body"},
                {"Short title", "Short body"});
    const auto saved = history.newest(0);
    return saved && saved->fMsg.fTitle == "Full title" &&
           saved->fMsg.fText == "Full body" &&
           saved->fCondensed.fTitle == "Short title" &&
           saved->fCondensed.fText == "Short body";
}

bool testRemoveUsesDisplayedOrdering() {
    eMessageHistory history;
    history.add({}, {"Oldest", ""});
    history.add({}, {"Middle", ""});
    history.add({}, {"Newest", ""});

    history.removeNewest(1);

    return history.size() == 2 &&
           history.newest(0)->fMsg.fTitle == "Newest" &&
           history.newest(1)->fMsg.fTitle == "Oldest";
}

bool testSidebarStateTracksCountAndLatestMessage() {
    eMessageHistory history;

    auto state = history.sidebarState();
    if(state.fCount != 0 || state.fLatest) return false;

    history.add({}, {"First", "First body"});
    state = history.sidebarState();
    if(state.fCount != 1 || !state.fLatest ||
       state.fLatest->fMsg.fTitle != "First") return false;

    history.add({}, {"Second", "Second body"});
    state = history.sidebarState();
    if(state.fCount != 2 || !state.fLatest ||
       state.fLatest->fMsg.fTitle != "Second") return false;

    history.removeNewest(0);
    state = history.sidebarState();
    return state.fCount == 1 && state.fLatest &&
           state.fLatest->fMsg.fTitle == "First";
}
}

int main() {
    return testEmptyHistory() &&
           testMessagesAreNewestFirst() &&
           testStoredMessagesCannotRetainLiveActionsOrLocations() &&
           testCondensedPreviewDoesNotReplaceFullMessage() &&
           testRemoveUsesDisplayedOrdering() &&
           testSidebarStateTracksCountAndLatestMessage() ? 0 : 1;
}
