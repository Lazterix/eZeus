#include "epopulationdata.h"

#include <algorithm>

ePopulationData::ePopulationData() {

}

void ePopulationData::incPopulation(const int p) {
    mPop = std::max(0, mPop + p);
}

void ePopulationData::incVacancies(const int v) {
    mVacs = std::max(0, mVacs + v);
}

void ePopulationData::incSettlers(const int s) {
    mSettlers = std::max(0, mSettlers + s);
    if(s > 0) mArrivedThisMonth += s;
}

void ePopulationData::incLeft(const int s) {
    if(s > 0) mLeftThisMonth += s;
}

void ePopulationData::nextMonth() {
    mArrivedThisMonth = 0;
    mLeftThisMonth = 0;
}
