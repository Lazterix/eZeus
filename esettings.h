#ifndef ESETTINGS_H
#define ESETTINGS_H

#include <array>

#include "widgets/eresolution.h"
#include "engine/etile.h"

struct eSettings {
    bool fTinyTextures = true;
    bool fSmallTextures = true;
    bool fMediumTextures = true;
    bool fLargeTextures = true;
    bool fFullscreen = false;
    eResolution fRes = eResolution(1280, 720);

    std::vector<eTileSize> availableSizes() const;
    bool validateTexturePacks(const std::array<bool, 4>& available);

    bool write() const;
    void read();
};

#endif // ESETTINGS_H
