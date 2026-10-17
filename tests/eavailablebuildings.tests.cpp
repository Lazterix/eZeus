#define SDL_MAIN_HANDLED

#include "buildings/eavailablebuildings.h"
#include "buildings/ebuilding.h"

bool eBuilding::sPyramidBuilding(const eBuildingType type) {
    const int id = static_cast<int>(type);
    return id >= static_cast<int>(eBuildingType::modestPyramid) &&
           id <= static_cast<int>(eBuildingType::pyramidMuseum);
}

#include "buildings/eavailablebuildings.cpp"

namespace {
bool testUnbuiltPyramidDoesNotLeakIntoNextEpisode() {
    eAvailableBuildings current;
    current.allowPyramid(eBuildingType::modestPyramid, {true, false});

    const eAvailableBuildings next;
    current.startEpisode(next);

    return !current.available(eBuildingType::modestPyramid) &&
           current.pyramidLevels(eBuildingType::modestPyramid).empty();
}

bool testNextEpisodePyramidReplacesPriorAvailability() {
    eAvailableBuildings current;
    current.allowPyramid(eBuildingType::modestPyramid, {true, false});

    eAvailableBuildings next;
    next.allowPyramid(eBuildingType::pyramid, {false, true, true});
    current.startEpisode(next);

    return !current.available(eBuildingType::modestPyramid) &&
           current.available(eBuildingType::pyramid) &&
           current.pyramidLevels(eBuildingType::pyramid) ==
               std::vector<bool>({false, true, true});
}

bool testBuiltPyramidSurvivesEpisodeTransition() {
    eAvailableBuildings current;
    current.allowPyramid(eBuildingType::modestPyramid, {true, false});
    current.built(eBuildingType::modestPyramid);

    const eAvailableBuildings next;
    current.startEpisode(next);
    current.destroyed(eBuildingType::modestPyramid);

    return current.available(eBuildingType::modestPyramid) &&
           current.pyramidLevels(eBuildingType::modestPyramid) ==
               std::vector<bool>({true, false});
}
}

int main() {
    return testUnbuiltPyramidDoesNotLeakIntoNextEpisode() &&
           testNextEpisodePyramidReplacesPriorAvailability() &&
           testBuiltPyramidSurvivesEpisodeTransition() ? 0 : 1;
}
