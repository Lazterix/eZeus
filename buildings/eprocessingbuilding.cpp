#include "eprocessingbuilding.h"

#include "characters/actions/ecarttransporteraction.h"
#include "textures/egametextures.h"
#include "engine/egameboard.h"
#include "enumbers.h"

#include <algorithm>
#include <cmath>

eProcessingBuilding::eProcessingBuilding(
        eGameBoard& board,
        const eBaseTex baseTex,
        const double overlayX,
        const double overlayY,
        const eOverlays overlays,
        const eBuildingType type,
        const int sw, const int sh,
        const int maxEmployees,
        const eResourceType rawMaterial,
        const eResourceType product,
        const int rawUse,
        const int time,
        const eCityId cid) :
    eResourceBuildingBase(board, type, sw, sh,
                          maxEmployees, product, cid),
    mTextures(eGameTextures::buildings()),
    mBaseTex(baseTex), mOverlays(overlays),
    mOverlayX(overlayX), mOverlayY(overlayY),
    mRawMaterial(rawMaterial), mRawUse(rawUse),
    mProcessWaitTime(time) {
    setOverlayEnabledFunc([this]() {
        return enabled() && mRawCount > 0;
    });
}

eProcessingBuilding::~eProcessingBuilding() {
    if(mTakeCart) mTakeCart->kill();
}

std::shared_ptr<eTexture> eProcessingBuilding::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    return mTextures[sizeId].*mBaseTex;
}

std::vector<eOverlay> eProcessingBuilding::getOverlays(
        const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& coll = mTextures[sizeId].*mOverlays;
    const int texId = textureTime() % coll.size();
    eOverlay o;
    o.fTex = coll.getTexture(texId);
    o.fX = mOverlayX;
    o.fY = mOverlayY;
    return std::vector<eOverlay>({o});
}

void eProcessingBuilding::timeChanged(const int by) {
    if(enabled()) {
        if(!mTakeCart) {
            mTakeCart = spawnCart(eCartActionTypeSupport::take);
            mTakeCart->setMaxDistance(eNumbers::sProcessingBuildingMaxResourceTakeDistance);
        }
        const auto type = resourceType();
        if(mRawCount >= mRawUse && spaceLeft(type) > 0) {
            mProcessTime += by*effectiveness();
            if(mProcessTime > mProcessWaitTime) {
                mProcessTime -= mProcessWaitTime;
                const int c = addProduced(type, 1);
                mRawCount -= c*mRawUse;
            }
        }
    }
    eResourceBuildingBase::timeChanged(by);
}

int eProcessingBuilding::count(const eResourceType type) const {
    if(type == mRawMaterial) return mRawCount;
    return eResourceBuildingBase::count(type);
}

int eProcessingBuilding::add(const eResourceType type, const int count) {
    if(type == mRawMaterial) {
        if(count <= 0) return 0;
        const int space = std::max(0, mMaxRaw - mRawCount);
        const int added = std::min(count, space);
        mRawCount += added;
        return added;
    }
    return eResourceBuildingBase::add(type, count);
}

int eProcessingBuilding::spaceLeft(const eResourceType type) const {
    if(type == mRawMaterial) return mMaxRaw - mRawCount;
    return eResourceBuildingBase::spaceLeft(type);
}

std::vector<eCartTask> eProcessingBuilding::cartTasks() const {
    auto tasks = eResourceBuildingBase::cartTasks();

    if(mMaxRaw > mRawCount) {
        eCartTask task;
        task.fType = eCartActionType::take;
        task.fResource = mRawMaterial;
        task.fMaxCount = mMaxRaw - mRawCount;
        tasks.push_back(task);
    }

    return tasks;
}

void eProcessingBuilding::read(eReadStream& src) {
    eResourceBuildingBase::read(src);

    src.readCharacter(&getBoard(), [this](eCharacter* const c) {
        mTakeCart = dynamic_cast<eCartTransporter*>(c);
    });

    src >> mRawCount;
    src >> mProcessTime;
    mRawCount = std::clamp(mRawCount, 0, mMaxRaw);
    if(!std::isfinite(mProcessTime) || mProcessTime < 0) {
        mProcessTime = 0;
    }
}

void eProcessingBuilding::write(eWriteStream& dst) const {
    eResourceBuildingBase::write(dst);

    dst.writeCharacter(mTakeCart);

    dst << mRawCount;
    dst << mProcessTime;
}
