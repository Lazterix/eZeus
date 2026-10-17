#ifndef ESIDEBARNOTIFICATIONSTATE_H
#define ESIDEBARNOTIFICATIONSTATE_H

#include <algorithm>
#include <cstddef>
#include <vector>

#include "engine/eevent.h"

enum class eSidebarAlertCategory {
    hazard,
    plague,
    invasion,
    arrival,
    none
};

inline eSidebarAlertCategory eSidebarAlertCategoryFor(const eEvent event) {
    switch(event) {
    case eEvent::fire:
    case eEvent::collapse:
    case eEvent::earthquake:
    case eEvent::earthquakeGod:
    case eEvent::tidalWave:
    case eEvent::tidalWaveGod:
    case eEvent::lavaFlow:
        return eSidebarAlertCategory::hazard;
    case eEvent::plague:
        return eSidebarAlertCategory::plague;
    case eEvent::godInvasion:
    case eEvent::playerGodAttack:
    case eEvent::monsterInvasion:
    case eEvent::godMonsterUnleash:
    case eEvent::monsterInCity:
    case eEvent::invasion:
    case eEvent::playerInvasion:
        return eSidebarAlertCategory::invasion;
    case eEvent::godVisit:
    case eEvent::godHelp:
    case eEvent::heroArrival:
    case eEvent::armyReturns:
    case eEvent::aidArrives:
        return eSidebarAlertCategory::arrival;
    default:
        return eSidebarAlertCategory::none;
    }
}

struct eSidebarAlertPushResult {
    bool fAccepted = false;
    int fId = -1;
    int fReplacedId = -1;
};

class eSidebarAlertState {
public:
    static constexpr int sLifetimeFrames = 200;

    eSidebarAlertPushResult push(const eEvent event, const int frame) {
        const auto category = eSidebarAlertCategoryFor(event);
        if(category == eSidebarAlertCategory::none) return {};

        eSidebarAlertPushResult result;
        result.fAccepted = true;
        const auto previous = std::find_if(
            mAlerts.begin(), mAlerts.end(),
            [category](const eAlert& alert) {
                return alert.fCategory == category;
            });
        if(previous != mAlerts.end()) {
            result.fReplacedId = previous->fId;
            mAlerts.erase(previous);
        }

        result.fId = mNextId++;
        mAlerts.push_back({result.fId, category, frame});
        return result;
    }

    std::vector<int> expire(const int frame) {
        std::vector<int> result;
        for(auto it = mAlerts.begin(); it != mAlerts.end();) {
            if(frame - it->fStartFrame >= sLifetimeFrames) {
                result.push_back(it->fId);
                it = mAlerts.erase(it);
            } else {
                ++it;
            }
        }
        return result;
    }

    bool dismiss(const int id) {
        const auto alert = std::find_if(
            mAlerts.begin(), mAlerts.end(),
            [id](const eAlert& entry) { return entry.fId == id; });
        if(alert == mAlerts.end()) return false;
        mAlerts.erase(alert);
        return true;
    }

    bool contains(const int id) const {
        return std::any_of(
            mAlerts.begin(), mAlerts.end(),
            [id](const eAlert& alert) { return alert.fId == id; });
    }

    void clear() { mAlerts.clear(); }
    bool empty() const { return mAlerts.empty(); }
    std::size_t size() const { return mAlerts.size(); }
private:
    struct eAlert {
        int fId;
        eSidebarAlertCategory fCategory;
        int fStartFrame;
    };

    int mNextId = 0;
    std::vector<eAlert> mAlerts;
};

struct eSidebarMessageLayout {
    int fY;
    int fHeight;
};

inline eSidebarMessageLayout eSidebarMessageLayoutFor(
        const int messageRowY, const int messageRowHeight,
        const int lowerControlsY) {
    const int y = messageRowY + messageRowHeight;
    return {y, std::max(0, lowerControlsY - y)};
}

inline bool eSidebarMessagePreviewVisible(
        const bool hasLatestMessage, const std::size_t) {
    return hasLatestMessage;
}

#endif // ESIDEBARNOTIFICATIONSTATE_H
