#ifndef ANTIBACKLASHLIB_ABCTESTSIMPORT_H
#define ANTIBACKLASHLIB_ABCTESTSIMPORT_H

#include <CDPSystem/Base/CDPPort.h>
#include <CDPSystem/Base/CDPProperty.h>

namespace AntiBacklashLib {

class SimCmdPort : public CDPPort
{
public:
    void Create(const char* shortName, CDPComponent* parent) override;

    CDPProperty<double> SpeedCMD;
    CDPProperty<bool> AntiBacklashEnabled;
    CDPProperty<bool> EnableFCs;
};

} // namespace AntiBacklashLib

#endif
