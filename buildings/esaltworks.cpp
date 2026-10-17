#include "esaltworks.h"

#include "textures/ebuildingtextures.h"
#include "textures/egametextures.h"

namespace {
constexpr int sProductionPeriod = 75000;
}

eSaltWorks::eSaltWorks(eGameBoard& board, const eCityId cid) :
    eResourceBuildingBase(board, eBuildingType::saltWorks,
                          2, 2, 10, eResourceType::salt, cid) {
    eGameTextures::loadSaltWorks();
}

std::shared_ptr<eTexture> eSaltWorks::getTexture(
        const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    return eGameTextures::buildings()[sizeId].fSaltWorks;
}

void eSaltWorks::timeChanged(const int by) {
    if(enabled()) {
        mProductionProgress += by*effectiveness();
        if(mProductionProgress > sProductionPeriod) {
            // Do not discard part of a completed batch when the yard is full.
            if(spaceLeft(eResourceType::salt) >= 4 &&
               addProduced(eResourceType::salt, 4) == 4) {
                mProductionProgress -= sProductionPeriod;
            } else {
                // Keep exactly one completed batch pending until there is room.
                mProductionProgress = sProductionPeriod;
            }
        }
    }
    eResourceBuildingBase::timeChanged(by);
}

void eSaltWorks::read(eReadStream& src) {
    eResourceBuildingBase::read(src);
    src >> mProductionProgress;
}

void eSaltWorks::write(eWriteStream& dst) const {
    eResourceBuildingBase::write(dst);
    dst << mProductionProgress;
}
