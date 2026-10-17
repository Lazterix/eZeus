#ifndef ESETTINGS_H
#define ESETTINGS_H

#include "widgets/eresolution.h"
#include "engine/etile.h"

struct eSettings {
    bool fTinyTextures = true;
    bool fSmallTextures = true;
    bool fMediumTextures = true;
    bool fLargeTextures = true;
    bool fFullscreen = false;
    bool fAutosave = true;
    bool fMusicEnabled = true;
    bool fSoundsEnabled = true;
    int fMusicVolume = 128;
    int fSoundsVolume = 128;
    eResolution fRes = eResolution(1280, 720);

    std::vector<eTileSize> availableSizes() const;

    void write() const;
    void read();
    void applyAudio() const;
};

#endif // ESETTINGS_H
