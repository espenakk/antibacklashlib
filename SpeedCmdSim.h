#ifndef ANTIBACKLASHLIB_SPEEDCMDTIMERSIM_H
#define ANTIBACKLASHLIB_SPEEDCMDTIMERSIM_H

#include <SimCmdPort.h>
#include <CDPAlarm/CDPAlarm.h>
#include <CDPParameter/CDPParameter.h>
#include <CDPSystem/Base/CDPComponent.h>
#include <Signal/CDPSignal.h>

namespace AntiBacklashLib {

class SpeedCmdSim : public CDPComponent
{
public:
    SpeedCmdSim();
    ~SpeedCmdSim() override;

    void Create(const char* fullName) override;
    void CreateModel() override;
    void Configure(const char* componentXML) override;
    void ProcessNull() override;
    void ProcessRunning();
    bool TransitionNullToRunning();
    bool TransitionRunningToNull();

protected:
    SimCmdPort SimCmd;
    CDPParameter RampDuration;
    CDPParameter ConstSpeedDuration;
    CDPParameter MaxSpeed;
    CDPSignal<bool> Start;

    CDPTimer timer;

    int testPhase;
    int testCycle;
    double originalMaxSpeed;
    double currentTestMaxSpeed;

    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;
};

} // namespace AntiBacklashLib

#endif
