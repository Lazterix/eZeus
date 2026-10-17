#ifndef EDISPLAYTRANSITION_H
#define EDISPLAYTRANSITION_H

#include "eresolution.h"

#include <SDL.h>

#include <string>
#include <vector>

struct eWindowedPlacement {
    int fX = SDL_WINDOWPOS_CENTERED;
    int fY = SDL_WINDOWPOS_CENTERED;
    int fWidth = 0;
    int fHeight = 0;
    bool fValid = false;
};

namespace eDisplayTransition {
std::vector<eResolution> availableResolutions(
    SDL_Window* window, const eResolution& preferred);

bool apply(SDL_Window* window,
           SDL_Renderer* renderer,
           const eResolution& resolution,
           bool fullscreen,
           eWindowedPlacement& windowedPlacement,
           std::string& error);
}

#endif // EDISPLAYTRANSITION_H
