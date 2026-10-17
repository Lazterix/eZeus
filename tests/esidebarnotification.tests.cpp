#define SDL_MAIN_HANDLED

#include "widgets/esidebarnotificationstate.h"
#include "widgets/emessagehistory.h"

eDate::eDate(const int day, const eMonth month, const int year) :
    mDay(day), mMonth(month), mYear(year) {}

namespace {
bool testOriginalAlertCategoriesReplaceTheirPreviousEntry() {
    eSidebarAlertState alerts;

    const auto fire = alerts.push(eEvent::fire, 0);
    const auto collapse = alerts.push(eEvent::collapse, 1);
    const auto plague = alerts.push(eEvent::plague, 2);
    const auto invasion = alerts.push(eEvent::invasion, 3);
    const auto arrival = alerts.push(eEvent::heroArrival, 4);

    return fire.fAccepted && fire.fReplacedId == -1 &&
           collapse.fAccepted && collapse.fReplacedId == fire.fId &&
           alerts.size() == 4 &&
           alerts.contains(collapse.fId) && alerts.contains(plague.fId) &&
           alerts.contains(invasion.fId) && alerts.contains(arrival.fId);
}

bool testTemporaryAlertsExpireOnUiFrames() {
    eSidebarAlertState alerts;
    const auto alert = alerts.push(eEvent::earthquake, 10);
    if(!alert.fAccepted) return false;

    if(!alerts.expire(10 + eSidebarAlertState::sLifetimeFrames - 1).empty()) {
        return false;
    }
    const auto expired =
        alerts.expire(10 + eSidebarAlertState::sLifetimeFrames);
    return expired.size() == 1 && expired.front() == alert.fId &&
           alerts.empty();
}

bool testExpiringAlertKeepsInboxHistory() {
    eMessageHistory history;
    history.add({}, {"Retained", "Message body"});

    eSidebarAlertState alerts;
    const auto alert = alerts.push(eEvent::aidArrives, 0);
    alerts.expire(eSidebarAlertState::sLifetimeFrames);
    return alert.fAccepted && alerts.empty() && history.size() == 1 &&
           history.newest(0)->fMsg.fTitle == "Retained";
}

bool testClickDismissesOnlyTheAlert() {
    eSidebarAlertState alerts;
    const auto alert = alerts.push(eEvent::aidArrives, 0);
    return alert.fAccepted && alerts.dismiss(alert.fId) && alerts.empty();
}

bool testNonSidebarEventsAreNotTracked() {
    eSidebarAlertState alerts;
    const auto request = alerts.push(eEvent::generalRequestAllyInitial, 0);
    return !request.fAccepted && alerts.empty();
}

bool testCondensedMessageBoxStaysBelowAlerts() {
    const auto layout = eSidebarMessageLayoutFor(239, 17, 282);
    return layout.fY == 256 && layout.fHeight == 26 &&
           layout.fHeight > 0 &&
           eSidebarMessagePreviewVisible(true, 4) &&
           !eSidebarMessagePreviewVisible(false, 4);
}
}

int main() {
    return testOriginalAlertCategoriesReplaceTheirPreviousEntry() &&
           testTemporaryAlertsExpireOnUiFrames() &&
           testExpiringAlertKeepsInboxHistory() &&
           testClickDismissesOnlyTheAlert() &&
           testNonSidebarEventsAreNotTracked() &&
           testCondensedMessageBoxStaysBelowAlerts() ? 0 : 1;
}
