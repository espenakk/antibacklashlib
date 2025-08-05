#ifndef ANTIBACKLASHLIB_FULLTEST_H
#define ANTIBACKLASHLIB_FULLTEST_H

#include <CDPAlarm/CDPAlarm.h>
#include <CDPParameter/CDPParameter.h>
#include <CDPSystem/Base/CDPComponent.h>
#include <Signal/CDPSignal.h>

namespace AntiBacklashLib {

class FullTest : public CDPComponent
{
public:
    FullTest();
    ~FullTest() override;

    void Create(const char* fullName) override;
    void CreateModel() override;
    void Configure(const char* componentXML) override;
    void ProcessNull() override;
    void ProcessRunning();
    bool TransitionNullToRunning();
    bool TransitionRunningToNull();

protected:
    CDPParameter NumTests;
    CDPSignal<int> AntiBacklashMode;
    CDPSignal<double> LoadTorqueLimit;
    CDPSignal<double> MaxTorque;
    CDPSignal<double> SlaveTorqueBase;
    CDPSignal<double> SlaveTorqueGain;
    CDPSignal<double> Offset;
    CDPSignal<double> SlaveDroop;
    CDPSignal<double> MasterDroop;
    CDPSignal<bool> Start;
    CDPSignal<double> MaxSpeed;
    CDPSignal<bool> Enable;
    CDPSignal<bool> IsRunning;
    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;


    int sequence;
    int currentMode;
    int currentParamStep;
};

} // namespace AntiBacklashLib

#endif
