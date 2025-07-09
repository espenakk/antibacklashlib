#ifndef ANTIBACKLASHLIB_ENCODERPORT_H
#define ANTIBACKLASHLIB_ENCODERPORT_H

#include <CDPSystem/Base/CDPPort.h>
#include <CDPSystem/Base/CDPProperty.h>

namespace AntiBacklashLib {

class EncoderPort : public CDPPort
{
public:
    void Create(const char* shortName, CDPComponent* parent) override;

    CDPProperty<double> position;
};

} // namespace AntiBacklashLib

#endif
