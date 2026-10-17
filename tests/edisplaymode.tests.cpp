#include "widgets/edisplaymode.h"

#include <algorithm>
#include <iostream>

namespace {
bool contains(const std::vector<eResolution>& values,
              const eResolution& expected) {
    return std::find(values.begin(), values.end(), expected) != values.end();
}

bool testModesAreFilteredDeduplicatedAndSorted() {
    const std::vector<eResolution> reported{
        {640, 480}, {1920, 1080}, {1920, 1080}, {3840, 2160}, {1366, 768}
    };
    const auto modes = eDisplayModeModel::availableResolutions(
        1920, 1080, reported, eResolution{1600, 900});

    return !contains(modes, {640, 480}) &&
           !contains(modes, {3840, 2160}) &&
           contains(modes, {800, 600}) &&
           contains(modes, {1366, 768}) &&
           contains(modes, {1600, 900}) &&
           contains(modes, {1920, 1080}) &&
           std::adjacent_find(modes.begin(), modes.end()) == modes.end() &&
           std::is_sorted(modes.begin(), modes.end(),
                          eDisplayModeModel::lessResolution);
}

bool testInvalidPreferenceFallsBackToSafeMode() {
    const std::vector<eResolution> modes{{800, 600}, {1024, 768}, {1280, 720}};
    return eDisplayModeModel::validatedResolution({0, 0}, modes) ==
               eResolution{1280, 720} &&
           eDisplayModeModel::validatedResolution({3000, 2000}, modes) ==
               eResolution{1280, 720};
}

bool testValidPreferenceIsPreserved() {
    const std::vector<eResolution> modes{{800, 600}, {1366, 768}, {1920, 1080}};
    return eDisplayModeModel::validatedResolution({1366, 768}, modes) ==
           eResolution{1366, 768};
}
}

int main() {
    return testModesAreFilteredDeduplicatedAndSorted() &&
           testInvalidPreferenceFallsBackToSafeMode() &&
           testValidPreferenceIsPreserved() ? 0 : 1;
}
