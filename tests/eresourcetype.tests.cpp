#define SDL_MAIN_HANDLED

#include "engine/eresourcetype.h"
#include "elanguage.h"
#include "textures/egametextures.h"

std::vector<eInterfaceTextures> eGameTextures::sInterfaceTextures;

const std::string& eLanguage::zeusText(const int, const int) {
    static const std::string text;
    return text;
}

namespace {
bool testOnlyIndividualResourceBitsAreSingleTypes() {
    return !eResourceTypeHelpers::isSingleType(eResourceType::none) &&
           eResourceTypeHelpers::isSingleType(eResourceType::wheat) &&
           eResourceTypeHelpers::isSingleType(eResourceType::salt) &&
           !eResourceTypeHelpers::isSingleType(eResourceType::food) &&
           !eResourceTypeHelpers::isSingleType(eResourceType::allTransportable);
}

bool testTransportCountsAreNormalizedForRestoredCarts() {
    return eResourceTypeHelpers::normalizedTransportCount(
               eResourceType::wheat, -1) == 0 &&
           eResourceTypeHelpers::normalizedTransportCount(
               eResourceType::wheat, 3) == 3 &&
           eResourceTypeHelpers::normalizedTransportCount(
               eResourceType::wheat, 5) == 4 &&
           eResourceTypeHelpers::normalizedTransportCount(
               eResourceType::sculpture, 2) == 1 &&
           eResourceTypeHelpers::normalizedTransportCount(
               eResourceType::food, 2) == 0 &&
           eResourceTypeHelpers::normalizedTransportCount(
               eResourceType::none, 2) == 0;
}
}

int main() {
    return testOnlyIndividualResourceBitsAreSingleTypes() &&
           testTransportCountsAreNormalizedForRestoredCarts() ? 0 : 1;
}
