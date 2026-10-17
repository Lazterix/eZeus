#include "etriremeaction.h"

#include "buildings/ebuilding.h"
#include "ekillcharacterfinishfail.h"
#include "characters/actions/emovetoaction.h"
#include "buildings/etriremewharf.h"
#include "characters/etrireme.h"
#include "engine/egameboard.h"

eTriremeAction::eTriremeAction(eTriremeWharf* const home,
                               eCharacter* const trireme) :
    eFightingAction(trireme, eCharActionType::triremeAction),
    mHome(home) {}

bool eTriremeAction::decide() {
    return true;
}

void eTriremeAction::increment(const int by) {
    const auto r = lookForEnemy(by);
    if(r == eLookForEnemyState::dead) return;
    eComplexAction::increment(by);
}

void eTriremeAction::read(eReadStream& src) {
    eFightingAction::read(src);
    auto& board = eTriremeAction::board();
    src.readBuilding(&board, [this](eBuilding* const b) {
        mHome = dynamic_cast<eTriremeWharf*>(b);
        // Away triremes are omitted from the wharf's saved ship reference.
        // Recover that state from this action's persisted home reference.
        if(mHome && !mHome->hasTrireme()) {
            mHome->triremeLeaving();
        }
    });
}

void eTriremeAction::write(eWriteStream& dst) const {
    eFightingAction::write(dst);
    dst.writeBuilding(mHome);
}

void eTriremeAction::goHome() {
    if(!mHome) return;
    const auto c = character();
    if(!c) return;

    const auto tile = mHome->triremeTile();
    if(!tile) return;
    const stdptr<eCharacter> cptr(c);

    const auto a = e::make_shared<eMoveToAction>(cptr.get());
    a->setStateRelevance(eStateRelevance::terrain);
    a->setFoundAction([cptr]() {
        if(!cptr) return;
        cptr->setActionType(eCharacterActionType::walk);
    });
    a->start(tile, eWalkableObject::sCreateDeepWater());
    setCurrentAction(a);
}

eTile* eTriremeAction::exitPoint() const {
    auto& board = eTriremeAction::board();
    const auto cid = cityId();
    return board.riverExitPoint(cid);
}

void eTriremeAction::goAbroad() {
    if(!mHome) return;
    const auto c = character();
    if(!c) return;

    const auto exitPoint = eTriremeAction::exitPoint();
    if(!exitPoint) return;

    auto& board = eFightingAction::board();
    const auto trireme = static_cast<eTrireme*>(c);
    board.deselectTrireme(trireme);
    mHome->triremeLeaving();
    const stdptr<eTrireme> cptr(trireme);
    const auto fail = std::make_shared<eKillCharacterFinishFail>(
        board, trireme);
    const auto finish = std::make_shared<eKillCharacterFinishFail>(
        board, trireme);

    const auto a = e::make_shared<eMoveToAction>(c);
    a->setStateRelevance(eStateRelevance::terrain);
    a->setFailAction(fail);
    a->setFinishAction(finish);
    a->setFindFailAction([cptr]() {
        if(cptr) cptr->kill();
    });
    setCurrentAction(a);
    c->setActionType(eCharacterActionType::walk);

    a->start(exitPoint, eWalkableObject::sCreateDeepWater());
}

eTriremeWharf *eTriremeAction::home() const {
    return mHome.get();
}
