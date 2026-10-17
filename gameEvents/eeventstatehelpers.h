#ifndef EEVENTSTATEHELPERS_H
#define EEVENTSTATEHELPERS_H

namespace eEventStateHelpers {
inline int normalizeCycleIndex(const int index, const int count) {
    return index >= 0 && index < count ? index : 0;
}

inline int nextCycleIndex(const int index, const int count) {
    const int current = normalizeCycleIndex(index, count);
    return current + 1 < count ? current + 1 : 0;
}

template<class Event, class Date>
inline void prepareEpisodeEvent(Event& event, const Date& date) {
    event.setIsEpisodeEvent(true);
    event.setupStartDate(date);
}

inline bool shouldRestoreRecurringEpisodeOwnership(
        const bool isRootEvent,
        const int remainingRuns,
        const bool episodeOwned) {
    return isRootEvent && remainingRuns > 1 && !episodeOwned;
}
}

#endif // EEVENTSTATEHELPERS_H
