#ifndef EDISPLAYMODE_H
#define EDISPLAYMODE_H

#include "eresolution.h"

#include <algorithm>
#include <vector>

namespace eDisplayModeModel {
inline bool lessResolution(const eResolution& lhs,
                           const eResolution& rhs) {
    const long long lhsArea = 1LL*lhs.width()*lhs.height();
    const long long rhsArea = 1LL*rhs.width()*rhs.height();
    if(lhsArea != rhsArea) return lhsArea < rhsArea;
    if(lhs.width() != rhs.width()) return lhs.width() < rhs.width();
    return lhs.height() < rhs.height();
}

inline std::vector<eResolution> availableResolutions(
        const int displayWidth,
        const int displayHeight,
        const std::vector<eResolution>& reported,
        const eResolution& preferred) {
    std::vector<eResolution> result;
    const auto add = [&](const eResolution& resolution) {
        if(resolution.width() < 800 || resolution.height() < 600) return;
        if(resolution.width() > displayWidth ||
           resolution.height() > displayHeight) return;
        if(std::find(result.begin(), result.end(), resolution) != result.end()) {
            return;
        }
        result.push_back(resolution);
    };

    for(const auto& resolution : eResolution::sResolutions) add(resolution);
    for(const auto& resolution : reported) add(resolution);
    add(preferred);

    std::sort(result.begin(), result.end(), lessResolution);
    return result;
}

inline eResolution validatedResolution(
        const eResolution& preferred,
        const std::vector<eResolution>& available) {
    if(std::find(available.begin(), available.end(), preferred) !=
       available.end()) return preferred;

    const eResolution safe{1280, 720};
    const auto safeIt = std::find(available.begin(), available.end(), safe);
    if(safeIt != available.end()) return *safeIt;
    if(!available.empty()) return available.front();
    return eResolution{800, 600};
}
}

#endif // EDISPLAYMODE_H
