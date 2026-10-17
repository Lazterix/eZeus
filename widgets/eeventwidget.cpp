#include "eeventwidget.h"

#include "engine/egameboard.h"
#include "engine/eevent.h"
#include "textures/egametextures.h"
#include "engine/eeventdata.h"
#include <algorithm>

void eEventWidget::pushEvent(const eEvent e, const eEventData& ed) {
    const auto state = mAlertState.push(e, mFrame);
    if(!state.fAccepted) return;
    if(state.fReplacedId >= 0) removeAlert(state.fReplacedId, false);

    const auto button = new eEventButton(e, window());
    mButtons.insert(mButtons.begin(), {button, state.fId});
    prependWidget(button);
    button->setPressAction([this, ed, id = state.fId]() {
        if(mViewTileHandler) {
            const auto ch = ed.fChar;
            if(ch) {
                const auto tile = ch->tile();
                mViewTileHandler(tile);
            } else {
                const auto tile = ed.fTile;
                mViewTileHandler(tile);
            }
        }

        removeAlert(id, true);
    });
    setHeight(button->height());
    setWidth(4*button->width());
    stackHorizontally();
    if(mEmptyChangedAction) mEmptyChangedAction(false);
}

void eEventWidget::incUiFrame() {
    mFrame++;
    const auto expired = mAlertState.expire(mFrame);
    for(const int id : expired) removeAlert(id, false);
}

void eEventWidget::clear() {
    removeChildren();
    mButtons.clear();
    mAlertState.clear();
    if(mEmptyChangedAction) mEmptyChangedAction(true);
}

void eEventWidget::setViewTileHandler(const eViewTileHandler& h) {
    mViewTileHandler = h;
}

void eEventWidget::removeAlert(const int id, const bool dismiss) {
    if(dismiss && !mAlertState.dismiss(id)) return;
    const auto entry = std::find_if(
        mButtons.begin(), mButtons.end(),
        [id](const eButtonState& state) { return state.fAlertId == id; });
    if(entry == mButtons.end()) return;

    entry->fButton->deleteLater();
    mButtons.erase(entry);
    stackHorizontally();
    if(mEmptyChangedAction) mEmptyChangedAction(mAlertState.empty());
}

eEventButton::eEventButton(const eEvent e,
                           eMainWindow* const window) :
    eButton(window) {
    const auto intrfc = eGameTextures::interface();
    const auto uiScale = resolution().uiScale();
    const int iRes = static_cast<int>(uiScale);
    const auto& texs = intrfc[iRes];
    const eTextureCollection* coll = nullptr;
    switch(e) {
    case eEvent::fire:
        coll = &texs.fFireAlert;
        break;
    case eEvent::collapse:
        coll = &texs.fCollapseAltert;
        break;
    case eEvent::earthquake:
    case eEvent::earthquakeGod:
        coll = &texs.fGroundFissureAlert;
        break;
    case eEvent::tidalWave:
    case eEvent::tidalWaveGod:
        coll = &texs.fFloodAlert;
        break;
    case eEvent::lavaFlow:
        coll = &texs.fLavaAltert;
        break;

    case eEvent::godVisit:
    case eEvent::godHelp:
        coll = &texs.fGodVisitAlert;
        break;

    case eEvent::godInvasion:
    case eEvent::playerGodAttack:
        coll = &texs.fGodAttackAlert;
        break;
    case eEvent::monsterInvasion:
    case eEvent::godMonsterUnleash:
    case eEvent::monsterInCity:
        coll = &texs.fMonsterAltert;
        break;
    case eEvent::heroArrival:
        coll = &texs.fHeroArrivalAlert;
        break;
    case eEvent::invasion:
    case eEvent::playerInvasion:
        coll = &texs.fInvasionAlert;
        break;
    case eEvent::plague:
        coll = &texs.fIllnessAlert;
        break;
    case eEvent::armyReturns:
    case eEvent::aidArrives:
        coll = &texs.fArmyComebackAlert;
        break;
    default:
        return;
    }

    setTexture(coll->getTexture(0));
    setHoverTexture(coll->getTexture(1));
    setPressedTexture(coll->getTexture(2));

    setNoPadding();
    fitContent();
}
