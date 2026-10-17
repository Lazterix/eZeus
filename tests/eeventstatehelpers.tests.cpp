#include "gameEvents/eeventstatehelpers.h"

namespace {
bool testValidIndicesPreserveOrdering() {
    return eEventStateHelpers::normalizeCycleIndex(0, 3) == 0 &&
           eEventStateHelpers::normalizeCycleIndex(1, 3) == 1 &&
           eEventStateHelpers::normalizeCycleIndex(2, 3) == 2 &&
           eEventStateHelpers::nextCycleIndex(0, 3) == 1 &&
           eEventStateHelpers::nextCycleIndex(1, 3) == 2 &&
           eEventStateHelpers::nextCycleIndex(2, 3) == 0;
}

bool testInvalidRestoredIndicesRestartAtFirstEvent() {
    return eEventStateHelpers::normalizeCycleIndex(-1, 3) == 0 &&
           eEventStateHelpers::normalizeCycleIndex(3, 3) == 0 &&
           eEventStateHelpers::normalizeCycleIndex(12, 3) == 0 &&
           eEventStateHelpers::normalizeCycleIndex(1, 0) == 0;
}

struct FakeEpisodeEvent {
    bool episodeOwned = false;
    int startDate = -1;
    int repeat = 12;
    int period = 365;

    void setIsEpisodeEvent(const bool owned) { episodeOwned = owned; }
    void setupStartDate(const int date) { startDate = date; }
    void startingNewEpisode() {
        if(episodeOwned) repeat = 0;
    }
};

bool testEpisodeEventsReceiveOwnershipBeforeScheduling() {
    FakeEpisodeEvent event;
    eEventStateHelpers::prepareEpisodeEvent(event, 42);

    const bool scheduledUnchanged = event.startDate == 42 &&
                                    event.repeat == 12 &&
                                    event.period == 365;
    event.startingNewEpisode();
    return scheduledUnchanged && event.episodeOwned && event.repeat == 0;
}

bool testOnlyRecurringRootRequestsQualifyForLegacyRepair() {
    return eEventStateHelpers::shouldRestoreRecurringEpisodeOwnership(
               true, 99998, false) &&
           !eEventStateHelpers::shouldRestoreRecurringEpisodeOwnership(
               true, 1, false) &&
           !eEventStateHelpers::shouldRestoreRecurringEpisodeOwnership(
               false, 99998, false) &&
           !eEventStateHelpers::shouldRestoreRecurringEpisodeOwnership(
               true, 99998, true);
}
}

int main() {
    return testValidIndicesPreserveOrdering() &&
           testInvalidRestoredIndicesRestartAtFirstEvent() &&
           testEpisodeEventsReceiveOwnershipBeforeScheduling() &&
           testOnlyRecurringRootRequestsQualifyForLegacyRepair() ? 0 : 1;
}
