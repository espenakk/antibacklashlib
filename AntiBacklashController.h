#ifndef ANTIBACKLASHLIB_ANTIBACKLASHCONTROLLER_H
#define ANTIBACKLASHLIB_ANTIBACKLASHCONTROLLER_H

#include <CDPAlarm/CDPAlarm.h>
#include <CDPParameter/CDPParameter.h>
#include <CDPSystem/Base/CDPComponent.h>
#include <Signal/CDPSignal.h>
#include <VaconLib/FCIOPort.h>
#include <VaconLib/VaconMarineAppFCPort.h>
#include <iostream>
#include <EncoderPort.h>

namespace AntiBacklashLib {

class AntiBacklashController : public CDPComponent
{
public:
    AntiBacklashController();
    ~AntiBacklashController() override;

    void Create(const char* fullName) override;
    void CreateModel() override;
    void Configure(const char* componentXML) override;
    void ProcessNull() override;

protected:
    VaconLib::VaconMarineAppFCPort FC1;
    VaconLib::VaconMarineAppFCPort FC2;
    VaconLib::VaconMarineAppFCPort FC3;
    EncoderPort ENC1;
    CDPParameter preload_torque;
    CDPParameter velocity_deadzone;
    CDPParameter userVelocityCmdA;
    CDPParameter userVelocityCmdB;
    CDPParameter userVelocityCmdC;
    CDPParameter enabled;
    CDPParameter loadEnabled;
    CDPParameter loadTorqueLimit;
    CDPParameter dir;
    CDPParameter antiBacklashEnabled;
    CDPSignal<bool> dirC;

    double positionSP;
    double kp_;
    double ki_;
    double kd_;

    double output;
    double error;
    double proportional;
    double integral;
    double derivative;
    double previousError;
    double accumulatedError;

    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;
};

} // namespace AntiBacklashLib

#endif
