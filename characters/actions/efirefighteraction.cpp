#include "efirefighteraction.h"

#include "../echaracter.h"
#include "engine/egameboard.h"
#include "emovetoaction.h"
#include "ewaitaction.h"

eFireFighterAction::eFireFighterAction(
        eCharacter* const c,
        ePatrolBuildingBase* const b,
        const std::vector<eOrientation>& path,
        const stdsptr<eDirectionTimes>& dirTimes) :
    ePatrolAction(c, b, path, dirTimes,
                  eCharActionType::fireFighterAction) {}

eFireFighterAction::eFireFighterAction(eCharacter* const c) :
    ePatrolAction(c, eCharActionType::fireFighterAction) {}

void eFireFighterAction::increment(const int by) {
    const auto c = character();
    if(!c || !c->tile()) {
        setState(eCharacterActionState::failed);
        return;
    }
    if(!mFireFighting) {
        const int fireCheckInc = 1000;
        mFireCheck += by;
        if(mFireCheck > fireCheckInc) {
            mFireCheck -= fireCheckInc;
            auto& brd = c->getBoard();
            const auto ct = c->tile();
            const int tx = ct->x();
            const int ty = ct->y();
            const int range = 10;
            bool found = false;
            for(int i = -range; i <= range && !found; i++) {
                for(int j = -range; j <= range && !found; j++) {
                    const int ttx = tx + i;
                    const int tty = ty + j;
                    const auto t = brd.tile(ttx, tty);
                    if(!t) continue;
                    found = t->onFire();
                }
            }
            if(found) lookForFire(false);
        }
    }
    ePatrolAction::increment(by);
}

eTile* neighbourOnFire(eTile* const tile, eOrientation& oo) {
    eTile* n = nullptr;
    for(const auto o : {eOrientation::topRight,
                        eOrientation::right,
                        eOrientation::bottomRight,
                        eOrientation::bottom,
                        eOrientation::bottomLeft,
                        eOrientation::left,
                        eOrientation::topLeft,
                        eOrientation::top}) {
        const auto nt = tile->neighbour<eTile>(o);
        if(!nt) continue;
        if(nt->onFire()) {
            oo = o;
            n = nt;
            break;
        }
    }
    return n;
}

bool eFireFighterAction::decide() {
    const auto c = character();
    if(!c || !c->tile()) {
        setState(eCharacterActionState::failed);
        return true;
    }
    if(mFireFighting) {
        if(mUsedWater >= 5) {
            c->setActionType(eCharacterActionType::walk);
            goBackDecision(eWalkableObject::sCreateDefault());
            return true;
        }
        const auto tile = c->tile();
        eOrientation oo;
        const auto n = neighbourOnFire(tile, oo);
        if(n) {
            c->setOrientation(oo);
            putOutFire(n);
            mUsedWater++;
        } else {
            c->setActionType(eCharacterActionType::stand);
            lookForFire(true);
        }
        return true;
    } else {
        const bool r = ePatrolAction::decide();
        if(r) return r;
    }

    return true;
}

void eFireFighterAction::read(eReadStream& src) {
    ePatrolAction::read(src);
    src >> mFireFighting;
    src >> mFireCheck;
    src >> mUsedWater;
}

void eFireFighterAction::write(eWriteStream& dst) const {
    ePatrolAction::write(dst);
    dst << mFireFighting;
    dst << mFireCheck;
    dst << mUsedWater;
}

bool eFireFighterAction::lookForFire(const bool second) {
    const auto c = character();
    if(!c) {
        setState(eCharacterActionState::failed);
        return false;
    }

    const auto failFunc = std::make_shared<eFFA_lookForFireFail>(
                              board(), this);

    const auto onFire = [](eTileBase* const tile) {
        return tile->onFire();
    };

    const auto a = e::make_shared<eMoveToAction>(c);
    a->setFailAction(failFunc);
    a->setMaxFindDistance(40);
    const stdptr<eFireFighterAction> tptr(this);
    a->setFoundAction([tptr, second, this]() {
        if(!tptr) return;
        if(second) {
            mFireFighting = true;
            const auto c = character();
            if(!c) {
                setState(eCharacterActionState::failed);
                return;
            }
            c->setActionType(eCharacterActionType::carry);
        } else {
            lookForFire(true);
        }
    });
    a->setRemoveLastTurn(true);

    a->setWait(mFireFighting || second);
    a->start(onFire);
    // Keep both stages alive through the parent action, not via a self-capture.
    setCurrentAction(a);

    return true;
}

void eFireFighterAction::putOutFire(eTile* const tile) {
    const auto c = character();
    c->setActionType(eCharacterActionType::fight);
    const auto finish = std::make_shared<eFFA_putOutFireFinish>(
                            board(), c, tile);
    const auto a = e::make_shared<eWaitAction>(c);
    a->setFinishAction(finish);
    if(tile->underBuilding()) {
        a->setTime(1600);
    } else {
        a->setTime(800);
    }
    setCurrentAction(a);
}
