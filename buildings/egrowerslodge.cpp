#include "egrowerslodge.h"

#include "textures/egametextures.h"
#include "characters/actions/ecarttransporteraction.h"
#include "characters/actions/egroweraction.h"
#include "enumbers.h"

#include <algorithm>

eGrowersLodge::eGrowersLodge(eGameBoard& board, const eGrowerType type,
                             const eCityId cid) :
    eEmployingBuilding(board,
                       type == eGrowerType::grapesAndOlives ?
                           eBuildingType::growersLodge :
                           eBuildingType::orangeTendersLodge, 2, 2, 12,
                       cid),
    mType(type) {
    switch(type) {
    case eGrowerType::grapesAndOlives:
        eGameTextures::loadGrowersLodge();
        break;
    case eGrowerType::oranges:
        eGameTextures::loadOrangeTendersLodge();
        break;
    }
}

eGrowersLodge::~eGrowersLodge() {
    if(mCart) mCart->kill();
    if(mGrower) mGrower->kill();
}

std::shared_ptr<eTexture> eGrowersLodge::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::buildings()[sizeId];
    switch(mType) {
    case eGrowerType::grapesAndOlives:
        return texs.fGrowersLodge;
    case eGrowerType::oranges:
        return texs.fOrangeTendersLodge;
    }
    return nullptr;
}

std::vector<eOverlay> eGrowersLodge::
    getOverlays(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::buildings()[sizeId];
    std::vector<eOverlay> os;

    switch(mType) {
    case eGrowerType::grapesAndOlives: {
        const auto& coll = texs.fGrowersLodgeOverlay;
        if(coll.size() <= 0) break;
        const int texId = textureTime() % coll.size();
        eOverlay o;
        o.fTex = coll.getTexture(texId);
        o.fX = -1.45;
        o.fY = -2.2;
        os.push_back(o);
    } break;
    case eGrowerType::oranges: {
        const auto& coll = texs.fOrangeTendersLodgeOverlay;
        if(coll.size() <= 0) break;
        const int texId = textureTime() % coll.size();
        eOverlay o;
        o.fTex = coll.getTexture(texId);
        o.fX = -1.75;
        o.fY = -2.3;
        os.push_back(o);
    } break;
    }

    if(mGrapes > 0) {
        eOverlay grapes;
        const auto& coll = texs.fWaitingGrapes;
        if(coll.size() <= 0) return os;
        const int resMax = coll.size() - 1;
        const int res = std::clamp(mGrapes - 1, 0, resMax);
        grapes.fTex = coll.getTexture(res);
        grapes.fX = -0.2;
        grapes.fY = -1.7;
        os.push_back(grapes);
    }
    if(mOlives > 0) {
        eOverlay olives;
        const auto& coll = texs.fWaitingOlives;
        if(coll.size() <= 0) return os;
        const int resMax = coll.size() - 1;
        const int res = std::clamp(mOlives - 1, 0, resMax);
        olives.fTex = coll.getTexture(res);
        olives.fX = -0.5;
        olives.fY = -1.85;
        os.push_back(olives);
    }
    if(mOranges > 0) {
        eOverlay oranges;
        const auto& coll = texs.fWaitingOranges;
        if(coll.size() <= 0) return os;
        const int resMax = coll.size() - 1;
        const int res = std::clamp(mOranges - 1, 0, resMax);
        oranges.fTex = coll.getTexture(res);
        oranges.fX = 0.00;
        oranges.fY = -1.80;
        os.push_back(oranges);
    }
    return os;
}

void eGrowersLodge::timeChanged(const int by) {
    if(enabled()) {
        const bool hasRes = mGrapes > 0 || mOlives > 0 || mOranges > 0;
        if(!mCart) {
            mCart = spawnCart(eCartActionTypeSupport::give);
        } else if(mCart && mCart->waiting() && hasRes) {
            // A partially loaded cart can only accept its existing product.
            const auto cargo = mCart->resCount() > 0 ?
                mCart->resType() : eResourceType::none;
            if(mGrapes > 0 &&
               (cargo == eResourceType::none || cargo == eResourceType::grapes)) {
                const int a = mCart->add(eResourceType::grapes, mGrapes);
                mGrapes -= a;
            } else if(mOlives > 0 &&
                      (cargo == eResourceType::none || cargo == eResourceType::olives)) {
                const int a = mCart->add(eResourceType::olives, mOlives);
                mOlives -= a;
            } else if(mOranges > 0 &&
                      (cargo == eResourceType::none || cargo == eResourceType::oranges)) {
                const int a = mCart->add(eResourceType::oranges, mOranges);
                mOranges -= a;
            }
        }
        if(mSpawnEnabled) {
            if(!mGrower) {
                const double eff = effectiveness();
                mSpawnTime += by*eff;
                if(mSpawnTime > eNumbers::sGrowerSpawnWaitTime) {
                    mSpawnTime = 0;
                    spawnGrower(&eGrowersLodge::mGrower);
                }
            }
        }
    }
    eEmployingBuilding::timeChanged(by);
}

int eGrowersLodge::add(const eResourceType type, const int count) {
    if(count <= 0) return 0;
    if(mType == eGrowerType::grapesAndOlives &&
       type != eResourceType::olives && type != eResourceType::grapes) return 0;
    if(mType == eGrowerType::oranges && type != eResourceType::oranges) return 0;
    const int sl = std::max(0, mMaxResource - mGrapes - mOlives - mOranges);
    const int r = std::min(count, sl);
    if(type == eResourceType::olives) {
        mOlives += r;
    } else if(type == eResourceType::grapes) {
        mGrapes += r;
    } else if(type == eResourceType::oranges) {
        mOranges += r;
    } else {
        return 0;
    }
    return r;
}

int eGrowersLodge::take(const eResourceType type, const int count) {
    if(type == eResourceType::olives) {
        const int result = std::clamp(count, 0, mOlives);
        mOlives -= result;
        return result;
    } else if(type == eResourceType::grapes) {
        const int result = std::clamp(count, 0, mGrapes);
        mGrapes -= result;
        return result;
    } else if(type == eResourceType::oranges) {
        const int result = std::clamp(count, 0, mOranges);
        mOranges -= result;
        return result;
    }
    return 0;
}

int eGrowersLodge::count(const eResourceType type) const {
    if(type == eResourceType::olives) {
        return mOlives;
    } else if(type == eResourceType::grapes) {
        return mGrapes;
    } else if(type == eResourceType::oranges) {
        return mOranges;
    }
    return 0;
}

int eGrowersLodge::spaceLeft(const eResourceType type) const {
    switch(mType) {
    case eGrowerType::grapesAndOlives:
        if(type != eResourceType::olives && type != eResourceType::grapes) return 0;
        break;
    case eGrowerType::oranges:
        if(type != eResourceType::oranges) return 0;
        break;
    }
    return std::max(0, mMaxResource - mOlives - mGrapes - mOranges);
}

std::vector<eCartTask> eGrowersLodge::cartTasks() const {
    std::vector<eCartTask> tasks;

    if(mGrapes > 0) {
        eCartTask task;
        task.fType = eCartActionType::give;
        task.fResource = eResourceType::grapes;
        task.fMaxCount = mGrapes;
        tasks.push_back(task);
    }

    if(mOlives > 0) {
        eCartTask task;
        task.fType = eCartActionType::give;
        task.fResource = eResourceType::olives;
        task.fMaxCount = mOlives;
        tasks.push_back(task);
    }

    if(mOranges > 0) {
        eCartTask task;
        task.fType = eCartActionType::give;
        task.fResource = eResourceType::oranges;
        task.fMaxCount = mOranges;
        tasks.push_back(task);
    }

    return tasks;
}

void eGrowersLodge::read(eReadStream& src) {
    eEmployingBuilding::read(src);
    src >> mNoTarget;
    src >> mSpawnEnabled;
    src >> mGrapes;
    src >> mOlives;
    src >> mOranges;
    src.readCharacter(&getBoard(), [this](eCharacter* const c) {
        mCart = dynamic_cast<eCartTransporter*>(c);
    });
    src >> mSpawnTime;
    src.readCharacter(&getBoard(), [this](eCharacter* const c) {
        mGrower = dynamic_cast<eGrower*>(c);
    });
}

void eGrowersLodge::write(eWriteStream& dst) const {
    eEmployingBuilding::write(dst);
    dst << mNoTarget;
    dst << mSpawnEnabled;
    dst << mGrapes;
    dst << mOlives;
    dst << mOranges;
    dst.writeCharacter(mCart);
    dst << mSpawnTime;
    dst.writeCharacter(mGrower);
}

bool eGrowersLodge::spawnGrower(const eGrowerPtr grower) {
    const auto t = centerTile();
    if(!t) return false;
    const auto g = e::make_shared<eGrower>(getBoard());
    g->setGrowerType(mType);
    g->setBothCityIds(cityId());
    g->changeTile(t);
    const auto a = e::make_shared<eGrowerAction>(
                       mType, this, g.get());
    g->setAction(a);
    this->*grower = g.get();
    return true;
}

void eGrowersLodge::setNoTarget(const bool t) {
    mNoTarget = t;
}
