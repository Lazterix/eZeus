#include "esettings.h"

#include <fstream>
#include <iostream>

#include "egamedir.h"
#include "eloadtexthelper.h"

#include <SDL_mixer.h>

#include <algorithm>

std::vector<eTileSize> eSettings::availableSizes() const {
    std::vector<eTileSize> sizes;
    if(fTinyTextures) {
        sizes.push_back(eTileSize::s15);
    }
    if(fSmallTextures) {
        sizes.push_back(eTileSize::s30);
    }
    if(fMediumTextures) {
        sizes.push_back(eTileSize::s45);
    }
    if(fLargeTextures) {
        sizes.push_back(eTileSize::s60);
    }
    return sizes;
}

void eSettings::write() const {
    const auto path = eGameDir::settingsPath();
    std::ofstream file;
    file.open(path);
    file << "tiny_textures" << " " <<
            (fTinyTextures ? "\"true\"" : "\"false\"") << "\n";
    file << "small_textures" << " " <<
            (fSmallTextures ? "\"true\"" : "\"false\"") << "\n";
    file << "medium_textures" << " " <<
            (fMediumTextures ? "\"true\"" : "\"false\"") << "\n";
    file << "large_textures" << " " <<
            (fLargeTextures ? "\"true\"" : "\"false\"") << "\n";
    file << "fullscreen" << " " <<
            (fFullscreen ? "\"true\"" : "\"false\"") << "\n";
    file << "autosave \"" << (fAutosave ? "true" : "false") << "\"\n";
    file << "music_enabled \"" << (fMusicEnabled ? "true" : "false") << "\"\n";
    file << "sounds_enabled \"" << (fSoundsEnabled ? "true" : "false") << "\"\n";
    file << "music_volume \"" << fMusicVolume << "\"\n";
    file << "sounds_volume \"" << fSoundsVolume << "\"\n";
    const auto wStr = std::to_string(fRes.width());
    file << "width" << " " << "\"" << wStr << "\"" << "\n";
    const auto hStr = std::to_string(fRes.height());
    file << "height" << " " << "\"" << hStr << "\"" << "\n";
    file.close();
}

void eSettings::read() {
    const auto path = eGameDir::settingsPath();
    std::map<std::string, std::string> settings;
    const bool r = eLoadTextHelper::load(path, settings);
    if(!r) return;

    const auto readBool = [&settings](const std::string& key,
                                      bool& value) {
        const auto it = settings.find(key);
        if(it == settings.end()) return;
        if(it->second == "true") value = true;
        else if(it->second == "false") value = false;
    };

    readBool("tiny_textures", fTinyTextures);
    readBool("small_textures", fSmallTextures);
    readBool("medium_textures", fMediumTextures);
    readBool("large_textures", fLargeTextures);
    readBool("fullscreen", fFullscreen);
    readBool("autosave", fAutosave);
    readBool("music_enabled", fMusicEnabled);
    readBool("sounds_enabled", fSoundsEnabled);

    const auto readVolume = [&settings](const std::string& key,
                                        int& value) {
        const auto it = settings.find(key);
        if(it == settings.end()) return;
        try {
            value = std::clamp(std::stoi(it->second), 0, 128);
        } catch(const std::exception&) {
        }
    };
    readVolume("music_volume", fMusicVolume);
    readVolume("sounds_volume", fSoundsVolume);

    const auto widthIt = settings.find("width");
    const auto heightIt = settings.find("height");
    const auto widthStr = widthIt == settings.end() ?
                              std::string{} : widthIt->second;
    const auto heightStr = heightIt == settings.end() ?
                               std::string{} : heightIt->second;
    if(!widthStr.empty() && !heightStr.empty()) {
        try {
            const int width = std::stoi(widthStr);
            const int height = std::stoi(heightStr);
            if(width > 0 && height > 0) {
                fRes = eResolution(width, height);
            }
        } catch(const std::exception&) {
            // Keep the existing/default resolution on malformed settings.
        }
    }
}

void eSettings::applyAudio() const {
    Mix_VolumeMusic(fMusicEnabled ? fMusicVolume : 0);
    Mix_Volume(-1, fSoundsEnabled ? fSoundsVolume : 0);
}

