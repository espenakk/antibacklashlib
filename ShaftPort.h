#ifndef ANTIBACKLASHLIB_SHAFTPORT_H
#define ANTIBACKLASHLIB_SHAFTPORT_H

#include <CDPSystem/Base/CDPPort.h>
#include <CDPSystem/Base/CDPProperty.h>

namespace AntiBacklashLib {

class ShaftPort : public CDPPort
{
public:
    void Create(const char* shortName, CDPComponent* parent) override;

    CDPProperty<short> ShaftAngle;
    CDPProperty<short> ShaftRounds;
};

} // namespace AntiBacklashLib

#endif
