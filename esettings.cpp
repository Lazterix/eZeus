#include "esettings.h"

#include <fstream>
#include <iostream>

#include "egamedir.h"
#include "eloadtexthelper.h"

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

bool eSettings::validateTexturePacks(
        const std::array<bool, 4>& available) {
    std::array<bool*, 4> enabled{
        &fTinyTextures,
        &fSmallTextures,
        &fMediumTextures,
        &fLargeTextures
    };

    bool found = false;
    for(std::size_t i = 0; i < enabled.size(); i++) {
        *enabled[i] = *enabled[i] && available[i];
        found = found || *enabled[i];
    }
    if(found) return true;

    constexpr std::array<std::size_t, 4> preference{1, 0, 2, 3};
    for(const auto i : preference) {
        if(!available[i]) continue;
        *enabled[i] = true;
        return true;
    }
    return false;
}

bool eSettings::write() const {
    const auto path = eGameDir::settingsPath();
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if(!file) {
        std::cerr << "Failed to open settings file for writing: '"
                  << path << "'.\n";
        return false;
    }
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
    const auto wStr = std::to_string(fRes.width());
    file << "width" << " " << "\"" << wStr << "\"" << "\n";
    const auto hStr = std::to_string(fRes.height());
    file << "height" << " " << "\"" << hStr << "\"" << "\n";
    file.close();
    if(file.fail()) {
        std::cerr << "Failed to write settings file: '" << path << "'.\n";
        return false;
    }
    return true;
}

void eSettings::read() {
    const auto path = eGameDir::settingsPath();
    std::map<std::string, std::string> settings;
    const bool r = eLoadTextHelper::load(path, settings);
    if(!r) return;
    fTinyTextures = settings["tiny_textures"] == "true";
    fSmallTextures = settings["small_textures"] == "true";
    fMediumTextures = settings["medium_textures"] == "true";
    fLargeTextures = settings["large_textures"] == "true";
    fFullscreen = settings["fullscreen"] == "true";
    const auto widthStr = settings["width"];
    const auto heightStr = settings["height"];
    if(!widthStr.empty() && !heightStr.empty()) {
        const int width = std::stoi(widthStr);
        const int height = std::stoi(heightStr);
        fRes = eResolution(width, height);
    }
}

