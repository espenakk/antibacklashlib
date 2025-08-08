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

    // --- Test Configuration ---
    static double constexpr GAIN_MIN{1},  GAIN_MAX{5};
    static double constexpr BASE_MIN{0.5},  BASE_MAX{2.0};
    static double constexpr OFFSET_MIN{0.25}, OFFSET_MAX{2.5};
    static double constexpr DROOP_MIN{1.0},  DROOP_MAX{10.0};
    static double constexpr DEGREE_MIN{230},  DEGREE_MAX{230};
    static double constexpr GAINDEG_MIN{0.001},  GAINDEG_MAX{0.01};
    static double constexpr DELAY_MIN{0.5},  DELAY_MAX{1.5};
};

} // namespace AntiBacklashLib

#endif
