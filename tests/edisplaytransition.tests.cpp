#define SDL_MAIN_HANDLED

#include "widgets/edisplaytransition.h"

#include <SDL.h>

#include <iostream>

namespace {
bool check(const bool condition, const char* const message) {
    if(condition) return true;
    std::cerr << message << '\n';
    return false;
}
}

int main() {
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << SDL_GetError() << '\n';
        return 1;
    }

    auto window = SDL_CreateWindow("eZeus display transition test",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   800, 600,
                                   SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    auto renderer = window ? SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_SOFTWARE) :
                             nullptr;
    if(!window || !renderer) {
        std::cerr << SDL_GetError() << '\n';
        if(renderer) SDL_DestroyRenderer(renderer);
        if(window) SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    eWindowedPlacement placement;
    std::string error;
    bool passed = eDisplayTransition::apply(
        window, renderer, {1024, 768}, false, placement, error);

    int width = 0;
    int height = 0;
    int logicalWidth = 0;
    int logicalHeight = 0;
    SDL_GetWindowSize(window, &width, &height);
    SDL_RenderGetLogicalSize(renderer, &logicalWidth, &logicalHeight);
    passed = check(passed && error.empty(),
                   "windowed transition failed") && passed;
    passed = check(width == 1024 && height == 768,
                   "windowed size was not applied") && passed;
    passed = check(logicalWidth == 1024 && logicalHeight == 768,
                   "logical size was not applied") && passed;

    error.clear();
    passed = check(eDisplayTransition::apply(
                       window, renderer, {1280, 720}, true,
                       placement, error),
                   "borderless fullscreen transition failed") && passed;
    passed = check((SDL_GetWindowFlags(window) &
                    SDL_WINDOW_FULLSCREEN_DESKTOP) ==
                       SDL_WINDOW_FULLSCREEN_DESKTOP,
                   "borderless fullscreen flag was not applied") && passed;
    SDL_RenderGetLogicalSize(renderer, &logicalWidth, &logicalHeight);
    passed = check(logicalWidth == 1280 && logicalHeight == 720,
                   "fullscreen logical size was not applied") && passed;

    error.clear();
    passed = check(eDisplayTransition::apply(
                       window, renderer, {1024, 768}, false,
                       placement, error),
                   "return to windowed mode failed") && passed;
    SDL_GetWindowSize(window, &width, &height);
    passed = check((SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) == 0,
                   "fullscreen flag survived windowed transition") && passed;
    passed = check(width == 1024 && height == 768,
                   "windowed size was not restored") && passed;

    error.clear();
    passed = check(eDisplayTransition::apply(
                       window, renderer, {1280, 720}, true,
                       placement, error),
                   "repeated fullscreen transition failed") && passed;
    passed = check(eDisplayTransition::apply(
                       window, renderer, {800, 600}, false,
                       placement, error),
                   "repeated windowed transition failed") && passed;
    SDL_GetWindowSize(window, &width, &height);
    SDL_RenderGetLogicalSize(renderer, &logicalWidth, &logicalHeight);
    passed = check(width == 800 && height == 600 &&
                       logicalWidth == 800 && logicalHeight == 600,
                   "repeated transition left stale geometry") && passed;

    error.clear();
    passed = check(!eDisplayTransition::apply(
                       window, renderer, {0, 0}, false,
                       placement, error) && !error.empty(),
                   "invalid resolution was accepted") && passed;
    SDL_GetWindowSize(window, &width, &height);
    passed = check(width == 800 && height == 600,
                   "invalid transition changed the window") && passed;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return passed ? 0 : 1;
}
