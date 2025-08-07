#ifndef ANTIBACKLASHLIB_FULLTEST_H
#define ANTIBACKLASHLIB_FULLTEST_H

#include <ABParamPort.h>
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
    ABParamPort ABParams;
    CDPParameter NumTests;
    CDPParameter GainMin;
    CDPParameter GainMax;
    CDPParameter BaseMin;
    CDPParameter BaseMax;
    CDPParameter OffsetMin;
    CDPParameter OffsetMax;
    CDPParameter DroopMin;
    CDPParameter DroopMax;

    CDPParameter AntiBacklashMode;
    CDPParameter LoadTorqueLimit;
    CDPParameter MaxTorque;
    CDPParameter SlaveTorqueBase;
    CDPParameter SlaveTorqueGain;
    CDPParameter Offset;
    CDPParameter SlaveDroop;
    CDPParameter MasterDroop;
    CDPParameter m_Start;
    CDPParameter m_MaxSpeed;

    CDPSignal<bool> Start;
    CDPSignal<double> MaxSpeed;
    CDPSignal<bool> Enable;
    CDPSignal<bool> IsRunning;
    CDPSignal<int> CurrentStatus;
    CDPSignal<int> TotalTests;
    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;


    int sequence;
    int currentMode;
    int currentParamStep;
};

} // namespace AntiBacklashLib

#endif
