#include "edisplaytransition.h"

#include "edisplaymode.h"

namespace {
struct eDisplayState {
    Uint32 fFlags = 0;
    int fX = 0;
    int fY = 0;
    int fWidth = 0;
    int fHeight = 0;
    int fLogicalWidth = 0;
    int fLogicalHeight = 0;
};

eDisplayState state(SDL_Window* const window,
                    SDL_Renderer* const renderer) {
    eDisplayState result;
    result.fFlags = SDL_GetWindowFlags(window);
    SDL_GetWindowPosition(window, &result.fX, &result.fY);
    SDL_GetWindowSize(window, &result.fWidth, &result.fHeight);
    SDL_RenderGetLogicalSize(renderer,
                             &result.fLogicalWidth,
                             &result.fLogicalHeight);
    return result;
}

bool fullscreen(const Uint32 flags) {
    return (flags & SDL_WINDOW_FULLSCREEN) != 0;
}

void restore(SDL_Window* const window,
             SDL_Renderer* const renderer,
             const eDisplayState& previous) {
    if(fullscreen(previous.fFlags)) {
        const Uint32 mode =
            (previous.fFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) ==
                    SDL_WINDOW_FULLSCREEN_DESKTOP ?
                SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
        SDL_SetWindowFullscreen(window, mode);
    } else {
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowSize(window, previous.fWidth, previous.fHeight);
        SDL_SetWindowPosition(window, previous.fX, previous.fY);
    }
    SDL_RenderSetLogicalSize(renderer,
                             previous.fLogicalWidth,
                             previous.fLogicalHeight);
}
}

std::vector<eResolution> eDisplayTransition::availableResolutions(
        SDL_Window* const window,
        const eResolution& preferred) {
    int display = window ? SDL_GetWindowDisplayIndex(window) : 0;
    if(display < 0) display = 0;

    SDL_Rect bounds{};
    if(SDL_GetDisplayBounds(display, &bounds) != 0) {
        bounds.w = 1280;
        bounds.h = 720;
    }

    std::vector<eResolution> reported;
    const int count = SDL_GetNumDisplayModes(display);
    for(int i = 0; i < count; ++i) {
        SDL_DisplayMode mode{};
        if(SDL_GetDisplayMode(display, i, &mode) == 0) {
            reported.emplace_back(mode.w, mode.h);
        }
    }

    SDL_DisplayMode desktop{};
    if(SDL_GetDesktopDisplayMode(display, &desktop) == 0) {
        reported.emplace_back(desktop.w, desktop.h);
    }

    return eDisplayModeModel::availableResolutions(
        bounds.w, bounds.h, reported, preferred);
}

bool eDisplayTransition::apply(
        SDL_Window* const window,
        SDL_Renderer* const renderer,
        const eResolution& resolution,
        const bool requestedFullscreen,
        eWindowedPlacement& windowedPlacement,
        std::string& error) {
    error.clear();
    if(!window || !renderer) {
        error = "display transition requires a window and renderer";
        return false;
    }
    if(resolution.width() < 800 || resolution.height() < 600) {
        error = "display resolution is below the supported minimum";
        return false;
    }

    const auto previous = state(window, renderer);
    const bool wasFullscreen = fullscreen(previous.fFlags);
    if(requestedFullscreen && !wasFullscreen) {
        windowedPlacement = {previous.fX, previous.fY,
                             previous.fWidth, previous.fHeight, true};
    }

    if(requestedFullscreen) {
        if(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
            error = SDL_GetError();
            restore(window, renderer, previous);
            return false;
        }
    } else {
        if(SDL_SetWindowFullscreen(window, 0) != 0) {
            error = SDL_GetError();
            restore(window, renderer, previous);
            return false;
        }
        SDL_SetWindowSize(window, resolution.width(), resolution.height());
        if(windowedPlacement.fValid) {
            SDL_SetWindowPosition(window,
                                  windowedPlacement.fX,
                                  windowedPlacement.fY);
        } else {
            SDL_SetWindowPosition(window,
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED);
        }
        int width = 0;
        int height = 0;
        SDL_GetWindowSize(window, &width, &height);
        if(width != resolution.width() || height != resolution.height()) {
            error = "SDL did not apply the requested window size";
            restore(window, renderer, previous);
            return false;
        }
    }

    if(SDL_RenderSetLogicalSize(renderer,
                                resolution.width(),
                                resolution.height()) != 0) {
        error = SDL_GetError();
        restore(window, renderer, previous);
        return false;
    }

    if(!requestedFullscreen) {
        SDL_GetWindowPosition(window,
                              &windowedPlacement.fX,
                              &windowedPlacement.fY);
        SDL_GetWindowSize(window,
                          &windowedPlacement.fWidth,
                          &windowedPlacement.fHeight);
        windowedPlacement.fValid = true;
    }
    return true;
}
