#include "egifthelpers.h"

namespace {
constexpr int giftCountValue(const eResourceType r) {
    switch(r) {
    case eResourceType::drachmas:
        return 500;
    case eResourceType::urchin:
    case eResourceType::fish:
    case eResourceType::meat:
    case eResourceType::cheese:
    case eResourceType::carrots:
    case eResourceType::onions:
    case eResourceType::wheat:
    case eResourceType::oranges:
    case eResourceType::food:
        return 8;
    case eResourceType::grapes:
    case eResourceType::olives:
    case eResourceType::wine:
    case eResourceType::oliveOil:
    case eResourceType::fleece:

    case eResourceType::wood:
    case eResourceType::bronze:
    case eResourceType::orichalc:
        return 8;
    case eResourceType::marble:
    case eResourceType::armor:
    case eResourceType::blackMarble:
        return 4;
    case eResourceType::sculpture:
        return 1;
    default:
        return 0;
    };
}

constexpr bool allGiftableResourcesHavePositiveCount() {
    const int allBasic = static_cast<int>(eResourceType::allBasic);
    for(int resource = 1; resource <= allBasic; resource <<= 1) {
        if(giftCountValue(static_cast<eResourceType>(resource)) <= 0) {
            return false;
        }
    }
    return giftCountValue(eResourceType::drachmas) > 0;
}

static_assert(giftCountValue(eResourceType::orichalc) == 8);
static_assert(giftCountValue(eResourceType::blackMarble) == 4);
static_assert(allGiftableResourcesHavePositiveCount());
}

int eGiftHelpers::giftCount(const eResourceType r) {
    return giftCountValue(r);
}
