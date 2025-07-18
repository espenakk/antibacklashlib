#ifndef ANTIBACKLASHLIB_ANTIBACKLASHCONTROLLER_H
#define ANTIBACKLASHLIB_ANTIBACKLASHCONTROLLER_H

#include <CDPAlarm/CDPAlarm.h>
#include <CDPParameter/CDPParameter.h>
#include <CDPSystem/Base/CDPComponent.h>
#include <Signal/CDPSignal.h>
#include <VaconLib/FCIOPort.h>
#include <VaconLib/VaconMarineAppFCPort.h>
#include <EncoderPort.h>

namespace AntiBacklashLib {

struct MotorRoles {
    VaconLib::VaconMarineAppFCPort* master;
    VaconLib::VaconMarineAppFCPort* slave;
    VaconLib::VaconMarineAppFCPort* load;
};

class AntiBacklashController : public CDPComponent
{
public:
    AntiBacklashController();
    ~AntiBacklashController() override;

    void Create(const char* fullName) override;
    void CreateModel() override;
    void Configure(const char* componentXML) override;

    void ProcessNull() override;
    void ProcessDebug();
    void ProcessPositionTest();
    void ProcessSpeedTimerTest();
    bool TransitionNullToDebug();
    bool TransitionDebugToNull();
    bool TransitionNullToPositionTest();
    bool TransitionPositionTestToNull();
    bool TransitionNullToSpeedTimerTest();
    bool TransitionSpeedTimerTestToNull();

protected:
    VaconLib::VaconMarineAppFCPort FC1;
    VaconLib::VaconMarineAppFCPort FC2;
    VaconLib::VaconMarineAppFCPort FC3;
    EncoderPort ENC1;
    CDPParameter loadTorqueLimit;
    CDPParameter maxTorque;
    CDPParameter maxSpeed;
    CDPParameter minSpeed;
    CDPParameter slowdownRange;
    CDPParameter degMargin;
    CDPParameter slaveTorqueBase;
    CDPParameter slaveTorqueGain;

    CDPSignal<bool> enabled;
    CDPSignal<bool> antiBacklashEnabled;
    CDPSignal<bool> startAntibacklashTestButton;
    CDPSignal<bool> debugMode;
    CDPSignal<double> speedCmd;
    CDPSignal<double> scaledEncSpeed;
    CDPSignal<double> scaledEncPosition;
    CDPSignal<double> FC1Speed;
    CDPSignal<double> FC2Speed;
    CDPSignal<double> FC1Torque;
    CDPSignal<double> FC2Torque;
    CDPSignal<double> FC3Torque;
    CDPSignal<double> FCSpeedRef;

    CDPTimer timer;
    CDPSignal<double> elapsedTime;

    MotorRoles motorRoles;

    double encSpeedScaler(const EncoderPort& rawEnc) { return double(rawEnc.speed) / 9.549297; }
    double encoderRawToDeg_F5888(const EncoderPort& rawEnc) { return static_cast<double>(rawEnc.position) * (360.0 / 65535); }
    void scaleFCSpeedTorque(MotorRoles& roles);
    void initFC(VaconLib::VaconMarineAppFCPort& FC, bool enable);
    double speedController(double errorDeg);

    double adaptiveSlaveTorque(double& speedCmd);
    
    MotorRoles chooseMasterSlave(double error);
    void setMasterSlaveTorque(MotorRoles& roles, double masterTorque, double slaveTorque);
    void setMasterSlaveSpeed(MotorRoles& roles, double masterSpeed, double slaveSpeed);
    void enableLoad(MotorRoles& roles, bool enable);
    void enableMasterSlave(MotorRoles& roles, bool enable);
    void stopAllMotors(MotorRoles& roles);

    double startPos;
    bool gotStartPos = false;

    double previousSpeedRef;

    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;
};

} // namespace AntiBacklashLib

#endif
