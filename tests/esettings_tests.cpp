#include "esettings.h"

#include <array>
#include <iostream>

namespace {

bool check(const bool condition, const char* const message) {
    if(condition) return true;
    std::cerr << message << '\n';
    return false;
}

bool unavailablePacksAreDisabled() {
    eSettings settings;
    settings.fTinyTextures = true;
    settings.fSmallTextures = true;
    settings.fMediumTextures = false;
    settings.fLargeTextures = false;

    const bool valid = settings.validateTexturePacks(
        std::array<bool, 4>{false, true, false, false});

    return check(valid, "an installed enabled pack should be valid") &&
           check(!settings.fTinyTextures,
                 "an unavailable texture pack remained enabled") &&
           check(settings.fSmallTextures,
                 "the installed enabled texture pack was disabled");
}

bool smallPackIsPreferredWhenRepairing() {
    eSettings settings;
    settings.fTinyTextures = false;
    settings.fSmallTextures = false;
    settings.fMediumTextures = false;
    settings.fLargeTextures = false;

    const bool valid = settings.validateTexturePacks(
        std::array<bool, 4>{true, true, true, true});

    return check(valid, "installed packs should produce a valid selection") &&
           check(!settings.fTinyTextures && settings.fSmallTextures &&
                     !settings.fMediumTextures && !settings.fLargeTextures,
                 "repair did not select only the installed small pack");
}

bool repairNeverEnablesAnUnavailablePack() {
    eSettings settings;
    settings.fTinyTextures = false;
    settings.fSmallTextures = false;
    settings.fMediumTextures = false;
    settings.fLargeTextures = false;

    const bool valid = settings.validateTexturePacks(
        std::array<bool, 4>{false, false, true, false});

    return check(valid, "the installed medium pack should be selected") &&
           check(!settings.fTinyTextures && !settings.fSmallTextures &&
                     settings.fMediumTextures && !settings.fLargeTextures,
                 "repair enabled an unavailable texture pack");
}

bool noInstalledPacksCannotBeRepaired() {
    eSettings settings;

    const bool valid = settings.validateTexturePacks(
        std::array<bool, 4>{false, false, false, false});

    return check(!valid, "missing texture packs should not validate") &&
           check(!settings.fTinyTextures && !settings.fSmallTextures &&
                     !settings.fMediumTextures && !settings.fLargeTextures,
                 "validation enabled a missing texture pack");
}

} // namespace

int main() {
    const bool passed = unavailablePacksAreDisabled() &&
                        smallPackIsPreferredWhenRepairing() &&
                        repairNeverEnablesAnUnavailablePack() &&
                        noInstalledPacksCannotBeRepaired();
    return passed ? 0 : 1;
}
