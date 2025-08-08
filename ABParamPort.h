#ifndef ANTIBACKLASHLIB_ABPARAMPORT_H
#define ANTIBACKLASHLIB_ABPARAMPORT_H

#include <CDPSystem/Base/CDPPort.h>
#include <CDPSystem/Base/CDPProperty.h>

namespace AntiBacklashLib {

class ABParamPort : public CDPPort
{
public:
    void Create(const char* shortName, CDPComponent* parent) override;

    CDPProperty<int> AntiBacklashMode;
    CDPProperty<double> LoadTorqueLimit;
    CDPProperty<double> MaxTorque;
    CDPProperty<double> SlaveTorqueBase;
    CDPProperty<double> SlaveTorqueGain;
    CDPProperty<double> Offset;
    CDPProperty<double> SlaveDroop;
    CDPProperty<double> MasterDroop;
    CDPProperty<double> SlaveDelay;
    CDPProperty<double> DegreeOffset;
    CDPProperty<double> DegreeGain;
};

} // namespace AntiBacklashLib

#endif
