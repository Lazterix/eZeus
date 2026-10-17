#include "ehorseranch.h"

#include "textures/egametextures.h"

#include "ehorseranchenclosure.h"
#include "engine/egameboard.h"
#include "enumbers.h"

#include <algorithm>

eHorseRanch::eHorseRanch(eGameBoard& board,
                         const eCityId cid) :
    eEmployingBuilding(board, eBuildingType::horseRanch, 3, 3, 15, cid) {
    eGameTextures::loadHorseRanch();
}

eHorseRanch::~eHorseRanch() {
    if(mTakeCart) mTakeCart->kill();
}

void eHorseRanch::erase() {
    if(mEnclosure) mEnclosure->eBuilding::erase();
    eBuilding::erase();
}

std::shared_ptr<eTexture> eHorseRanch::getTexture(
        const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings();
    return blds[sizeId].fHorseRanch;
}

std::vector<eOverlay> eHorseRanch::getOverlays(
        const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& blds = eGameTextures::buildings();
    const auto& texs = blds[sizeId];
    const auto& coll = texs.fHorseRanchOverlay;
    if(coll.size() <= 0) return {};
    const int texId = textureTime() % coll.size();

    std::vector<eOverlay> os;
    eOverlay& o = os.emplace_back();
    o.fTex = coll.getTexture(texId);
    o.fX = -2.1;
    o.fY = -2.85;

    if(mWheat > 0) {
        const int nw = std::clamp((mWheat - 50)/100, 0, 7);
        eOverlay& wo = os.emplace_back();
        wo.fTex = texs.fWaitingWheat.getTexture(nw);
        wo.fX = -0.4;
        wo.fY = -2.7;
    }

    return os;
}

void eHorseRanch::timeChanged(const int by) {
    if(enabled()) {
        if(!mTakeCart) {
            mTakeCart = spawnCart(eCartActionTypeSupport::take);
        }
        if(mWheat > 0) {
            mWheatTime += by;
            if(mWheatTime > eNumbers::sHorseRanchWheatUsePeriod) {
                mWheatTime = 0;
                // Small remaining portions must not turn into negative feed.
                mWheat = std::max(0, mWheat - 10);
            }
            mHorseTime += by;
            if(mHorseTime >= eNumbers::sHorseRanchHorseSpawnPeriod) {
                const bool produced = mEnclosure && mEnclosure->spawnHorse();
                // Do not waste a finished breeding cycle while the pen is full.
                mHorseTime = produced ? 0 : eNumbers::sHorseRanchHorseSpawnPeriod;
                if(produced && isPersonPlayer()) {
                    auto& board = getBoard();
                    board.incProduced(eResourceType::horse, 1);
                }
            }
        }
    }
    eEmployingBuilding::timeChanged(by);
}

int eHorseRanch::count(const eResourceType type) const {
    if(type == eResourceType::wheat) return mWheat/100;
    if(type == eResourceType::horse) return horseCount();
    return eEmployingBuilding::count(type);
}

int eHorseRanch::add(const eResourceType type, const int count) {
    if(type == eResourceType::wheat) {
        const int accepted = std::clamp(count, 0, spaceLeft(type));
        mWheat += accepted*100;
        return accepted;
    }
    return eEmployingBuilding::add(type, count);
}

int eHorseRanch::take(const eResourceType type, const int count) {
    if(type == eResourceType::horse) {
        return mEnclosure ? mEnclosure->take(type, count) : 0;
    }
    return eEmployingBuilding::take(type, count);
}

int eHorseRanch::spaceLeft(const eResourceType type) const {
    // Wheat consumption happens in smaller portions than a delivery.
    // Reserve room for the full 100-unit incoming batch.
    if(type == eResourceType::wheat) return std::max(0, (800 - mWheat)/100);
    return eEmployingBuilding::spaceLeft(type);
}

std::vector<eCartTask> eHorseRanch::cartTasks() const {
    auto tasks = eEmployingBuilding::cartTasks();

    const int room = spaceLeft(eResourceType::wheat);
    if(room > 0) {
        eCartTask task;
        task.fType = eCartActionType::take;
        task.fResource = eResourceType::wheat;
        task.fMaxCount = room;
        tasks.push_back(task);
    }

    return tasks;
}

int eHorseRanch::provide(const eProvide p, const int n) {
    if(mEnclosure) mEnclosure->eBuildingWithResource::provide(p, n);
    return eEmployingBuilding::provide(p, n);
}

void eHorseRanch::setEnclosure(eHorseRanchEnclosure* const e) {
    mEnclosure = e;
}

eHorseRanchEnclosure* eHorseRanch::enclosure() const {
    return mEnclosure.get();
}

int eHorseRanch::horseCount() const {
    return mEnclosure ? mEnclosure->horseCount() : 0;
}

bool eHorseRanch::takeHorse() {
    return mEnclosure && mEnclosure->takeHorse();
}

void eHorseRanch::read(eReadStream& src) {
    eEmployingBuilding::read(src);

    src >> mWheat;
    src >> mWheatTime;
    src >> mHorseTime;
    mWheat = std::max(0, mWheat);
    mWheatTime = std::max(0, mWheatTime);
    mHorseTime = std::max(0, mHorseTime);
    auto& board = getBoard();
    src.readCharacter(&board, [this](eCharacter* const c) {
        mTakeCart = dynamic_cast<eCartTransporter*>(c);
    });
}

void eHorseRanch::write(eWriteStream& dst) const {
    eEmployingBuilding::write(dst);

    dst << mWheat;
    dst << mWheatTime;
    dst << mHorseTime;
    dst.writeCharacter(mTakeCart);
}
