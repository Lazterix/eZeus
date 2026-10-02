#ifndef ESALTWORKS_H
#define ESALTWORKS_H

#include "eresourcebuildingbase.h"

class eSaltWorks : public eResourceBuildingBase {
public:
    eSaltWorks(eGameBoard& board, const eCityId cid);

    std::shared_ptr<eTexture> getTexture(const eTileSize size) const override;
    void timeChanged(const int by) override;

    void read(eReadStream& src) override;
    void write(eWriteStream& dst) const override;
private:
    double mProductionProgress = 0;
};

#endif // ESALTWORKS_H
