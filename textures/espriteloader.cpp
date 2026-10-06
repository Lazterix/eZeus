#include "espriteloader.h"

#include <cstdlib>
#include <filesystem>

namespace {

bool textureOverridesDisabled() {
    const char* const value = std::getenv("EZEUS_DISABLE_TEXTURE_OVERRIDES");
    if(!value) return false;

    const std::string v(value);
    return v == "1" ||
           v == "true" ||
           v == "TRUE" ||
           v == "yes" ||
           v == "YES";
}

}

void eSpriteLoader::loadTrailer(const int doff,
                                const int min, const int max,
                                eTextureCollection& coll, const int dy) {
    loadSkipFlipped(doff, min, max, coll);
    for(int i = 0; i < max - min; i++) {
        const auto& tex = coll.getTexture(i);
        tex->setOffset(tex->offsetX(), tex->offsetY() + dy);
    }
}

void eSpriteLoader::loadArrowSkipFlipped(const int doff,
                                         const int min, const int max,
                                         eTextureCollection& coll) {
    for(int i = min; i < max; i++) {
        if(i - min > 15 && i - min < 31) {
            auto& tex = coll.addTexture();
            const auto& flipTex = coll.getTexture(min + 30 - i);
            tex->setFlipTex(flipTex);
            if(mOffs) {
                const auto& offset = (*mOffs)[i - 1];
                tex->setOffset(offset.first, offset.second);
            }
        } else {
            load(doff, i, coll);
        }
    }
}

void eSpriteLoader::loadSkipFlipped(const int doff,
                                    const int min, const int max,
                                    eTextureCollection& coll) {
    for(int i = min; i < max;) {
        for(int j = 0; j < 8; j++, i++) {
            if(j > 3 && j < 7) {
                auto& tex = coll.addTexture();
                const auto& flipTex = coll.getTexture(6 - j);
                tex->setFlipTex(flipTex);
                if(mOffs) {
                    const auto& offset = (*mOffs)[i - 1];
                    tex->setOffset(offset.first, offset.second);
                }
            } else {
                load(doff, i, coll);
            }
        }
    }
}

void eSpriteLoader::loadSkipFlipped(const int doff,
                                    const int min, const int max,
                                    std::vector<eTextureCollection>& colls) {
    for(int j = 0; j < 8; j++) {
        colls.emplace_back(mRenderer);
    }
    int k = 0;
    for(int i = min; i < max;) {
        for(int j = 0; j < 8; j++, i++) {
            auto& coll = colls[j];
            if(j > 3 && j < 7) {
                const auto& flipTex = colls[6 - j].getTexture(k);
                auto& tex = coll.addTexture();
                tex->setFlipTex(flipTex);
                if(mOffs) {
                    const auto& offset = (*mOffs)[i - 1];
                    tex->setOffset(offset.first, offset.second);
                }
            } else {
                load(doff, i, coll);
            }
        }
        k++;
    }
}

void eSpriteLoader::loadHorseSkipFlipped(const int doff,
                                         const int min, const int max,
                                         std::vector<eTextureCollection> &colls) {
    for(int j = 0; j < 16; j++) {
        colls.emplace_back(mRenderer);
    }
    int k = 0;
    for(int i = min; i < max;) {
        for(int j = 0; j < 16; j++, i++) {
            auto& coll = colls[j];
            if(j > 7 && j < 15) {
                const auto& flipTex = colls[14 - j].getTexture(k);
                auto& tex = coll.addTexture();
                tex->setFlipTex(flipTex);
                if(mOffs) {
                    const auto& offset = (*mOffs)[i - 1];
                    tex->setOffset(offset.first, offset.second);
                }
            } else {
                load(doff, i, coll);
            }
        }
        k++;
    }
}

void eSpriteLoader::loadBoatSkipFlipped(const int doff,
                                        const int min, const int max,
                                        std::vector<eTextureCollection>& colls) {
    for(int j = 0; j < 8; j++) {
        colls.emplace_back(mRenderer);
    }
    int k = 0;
    for(int i = min; i < max;) {
        for(int j = 0; j < 8; j++, i += 2) {
            auto& coll = colls[j];
            if(j > 3 && j < 7) {
                const auto& flipTex = colls[6 - j].getTexture(k);
                auto& tex = coll.addTexture();
                tex->setFlipTex(flipTex);
                if(mOffs) {
                    const auto& offset = (*mOffs)[i - 1];
                    tex->setOffset(offset.first, offset.second);
                }
            } else {
                load(doff, i, coll);
            }
        }
        k++;
    }
}

const std::shared_ptr<eTexture>& eSpriteLoader::load(
        const int doff, const int i, eTextureCollection& coll) {
    const auto& sd = mSds[i - doff];
    const int tid = sd.fTexId;
    const auto t = tid == -1 ? nullptr : getTex(tid);
    const SDL_Rect rect{sd.fX, sd.fY, sd.fW, sd.fH};
    const auto& tex = coll.addTexture();
    tex->setParentTexture(rect, t);
    if(mOffs) {
        const auto& off = (*mOffs)[i - 1];
        tex->setOffset(off.first, off.second);
    }
    return tex;
}

std::shared_ptr<eTexture> eSpriteLoader::load(
        const int doff, const int i) {
    const auto& sd = mSds[i - doff];
    const int tid = sd.fTexId;
    const auto t = getTex(tid);
    std::shared_ptr<eTexture> tex;
    if(mSds.size() == 1) {
        tex = t;
    } else {
        const SDL_Rect rect{sd.fX, sd.fY, sd.fW, sd.fH};
        tex = std::make_shared<eTexture>();
        tex->setParentTexture(rect, t);
    }
    if(mOffs) {
        const auto& off = (*mOffs)[i - 1];
        tex->setOffset(off.first, off.second);
    }
    return tex;
}

void eSpriteLoader::loadTex(const int i) {
    const std::string relativePath =
            mSize + "/" + mName + "_" + std::to_string(i) + ".png";

    std::shared_ptr<eTexture> tex;

    if(!textureOverridesDisabled()) {
        const std::string overridePath =
                eGameDir::texturesDir() + relativePath;

        std::error_code ec;
        if(std::filesystem::is_regular_file(overridePath, ec) && !ec) {
            const auto overrideTex = std::make_shared<eTexture>();
            if(overrideTex->load(mRenderer, overridePath)) {
                printf("Texture override: %s\n", overridePath.c_str());
                tex = overrideTex;
            } else {
                printf("Texture override failed, using packed asset: %s\n",
                       overridePath.c_str());
            }
        }
    }

    if(!tex) {
        tex = eBinaryImageLoader::load(mRenderer, relativePath);
    }

    mTexs[i] = tex;
}

const std::shared_ptr<eTexture>& eSpriteLoader::getTex(const int i) {
    if(mTexs.find(i) == mTexs.end()) {
        loadTex(i);
    }
    return mTexs[i];
}
