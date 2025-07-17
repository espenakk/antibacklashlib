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
    void ProcessRunning();
    bool TransitionNullToDebug();
    bool TransitionDebugToNull();
    bool TransitionNullToRunning();
    bool TransitionRunningToNull();

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
    CDPParameter slaveTorque;
    CDPParameter masterDroop;
    CDPParameter slaveDroop;
    CDPParameter loadDroop;

    CDPSignal<bool> enabled;
    CDPSignal<bool> loadEnabled;
    CDPSignal<bool> antiBacklashEnabled;
    CDPSignal<bool> dir;
    CDPSignal<bool> dirC;
    CDPSignal<bool> startAntibacklashTestButton;
    CDPSignal<bool> runningAntiBacklashTestScript;
    CDPSignal<bool> debugMode;
    CDPSignal<double> speedCmdA;
    CDPSignal<double> speedCmdB;
    CDPSignal<double> speedCmdC;
    CDPSignal<double> scaledEncSpeed;
    CDPSignal<double> scaledEncPosition;

    CDPTimer timer;
    CDPSignal<double> elapsedTime;

    MotorRoles motorRoles;

    double encSpeedScaler(const EncoderPort& rawEnc) { return double(rawEnc.speed) / 9.549297; }
    double EncoderRawToDeg_F5888(const EncoderPort& rawEnc) { return static_cast<double>(rawEnc.position) * (360.0 / 65535); }

    void initFC(VaconLib::VaconMarineAppFCPort& FC, bool enable);
    double SpeedController(double errorDeg);
    void MoveToPos(MotorRoles& motorRoles, double targetDeg, bool antiBacklashEnabled);
    
    MotorRoles ChooseMasterSlave(double errorDeg);
    void setMasterSlaveTorque(MotorRoles& roles, double masterTorque, double slaveTorque);
    void setMasterSlaveSpeed(MotorRoles& roles, double masterSpeed, double slaveSpeed);
    void enableLoad(MotorRoles& roles, bool enable);
    void enableMasterSlave(MotorRoles& roles, bool enable);
    void StopAllMotors(MotorRoles& roles);
    void setLoadDrooping(MotorRoles& roles, double masterDroop, double slaveDroop, double loadDroop);

    double startPos;
    bool gotStartPos = false;

    double preloadTorque = 0.15 * maxTorque; //Nm

    // static constexpr double kLoadTorqueLimit = 2.0; //Nm
    // static constexpr double kMaxTorque = 10; //Nm
    // static constexpr double kMaxSpeed = 2.0 * M_PI / 5;
    // static constexpr double kMinSpeed = 2.0 * M_PI / 20;
    // static constexpr double kSlowdownRange = 50.0;
    // static constexpr double kDegMargin = 10.0;

    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;
};

} // namespace AntiBacklashLib

#endif
