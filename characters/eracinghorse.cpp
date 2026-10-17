#include "eracinghorse.h"

eRacingHorse::eRacingHorse(eGameBoard& board, const int id,
                           const std::vector<ePathPoint>& path) :
    eMissile(board, eMissileType::racingHorse, path),
    mId(id) {
    eGameTextures::loadRacingHorses();
}

eRacingHorse::eRacingHorse(eGameBoard& board) :
    eRacingHorse(board, 0, {}) {}

std::shared_ptr<eTexture>
eRacingHorse::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& ctexs = eGameTextures::characters()[sizeId];
    const eRacingHorseTextures* texs = nullptr;
    const int id = mId % 4;
    if(id == 0) {
        texs = &ctexs.fRacingHorse1;
    } else if(id == 1) {
        texs = &ctexs.fRacingHorse2;
    } else if(id == 2) {
        texs = &ctexs.fRacingHorse3;
    } else if(id == 3) {
        texs = &ctexs.fRacingHorse4;
    }
    if(!texs) return nullptr;
    const auto& race = texs->fRace;
    const double a = angle();
    const auto& board = eMissile::board();
    const auto dir = board.direction();
    double da;
    if(dir == eWorldDirection::N) {
        da = 225;
    } else if(dir == eWorldDirection::E) {
        da = 315;
    } else if(dir == eWorldDirection::S) {
        da = 45;
    } else { // if(dir == eWorldDirection::W) {
        da = 135;
    }
    const int raceSize = static_cast<int>(race.size());
    if(raceSize <= 0) return nullptr;

    const int rawAid = static_cast<int>(
        std::round((a - da)/22.5) - 1);
    const int aid = (rawAid % raceSize + raceSize) % raceSize;
    const auto& coll = race[aid];
    if(coll.size() <= 0) return nullptr;

    const int time = 5*textureTime();
    return coll.getTexture(time % coll.size());
}

void eRacingHorse::read(eReadStream& src) {
    eMissile::read(src);
    src >> mId;
}

void eRacingHorse::write(eWriteStream& dst) const {
    eMissile::write(dst);
    dst << mId;
}
