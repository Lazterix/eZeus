#include "etroopsrequestfulfilledevent.h"

#include "elanguage.h"
#include "estringhelpers.h"
#include "engine/eeventdata.h"
#include "engine/egameboard.h"
#include "etroopsrequestevent.h"

#include <algorithm>

eTroopsRequestFulfilledEvent::eTroopsRequestFulfilledEvent(
        const eCityId cid,
        const eGameEventBranch branch,
        eGameBoard& board) :
    ePlayerConquestEventBase(cid, eGameEventType::troopsRequestFulfilled,
                             branch, board) {}

void eTroopsRequestFulfilledEvent::initialize(
        const eEnlistedForces& forces,
        const stdsptr<eWorldCity>& city,
        const stdsptr<eWorldCity>& rivalCity) {
    mForces = forces;
    mCity = city;
    mRivalCity = rivalCity;
}

void eTroopsRequestFulfilledEvent::trigger() {
    removeArmyEvent();
    removeConquestEvent();
    if(!mCity) return;

    auto rivalCity = mRivalCity;
    const auto me = mainEvent<eTroopsRequestEvent>();
    if(!rivalCity && me) {
        rivalCity = me->attackingCity();
        mRivalCity = rivalCity;
    }
    if(!rivalCity) {
        planArmyReturn();
        return;
    }

    const int enemyStr = rivalCity->troops();
    const int str = mCity->troops() + mForces.strength();

    if(str > 0) {
        const double killFrac = std::clamp(0.5*enemyStr/str, 0., 1.);
        mForces.kill(killFrac);
        const int t = mCity->troops();
        mCity->setTroops((1 - 0.5*killFrac)*t);
    }

    const bool defended = str > enemyStr;

    if(defended) {
        if(me) me->won();
    } else {
        if(me) me->lost();
    }

    planArmyReturn();
}

std::string eTroopsRequestFulfilledEvent::longName() const {
    return eLanguage::zeusText(290, 6);
}
