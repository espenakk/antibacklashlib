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

struct MasterSlave {
    VaconLib::VaconMarineAppFCPort* master;
    VaconLib::VaconMarineAppFCPort* slave;
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

    CDPSignal<bool> enabled;
    CDPSignal<bool> loadEnabled;
    CDPSignal<bool> antiBacklashEnabled;
    CDPSignal<bool> dir;
    CDPSignal<bool> dirC;
    CDPSignal<bool> startAntibacklashTestButton;
    CDPSignal<bool> runningAntiBacklashTestScript;
    CDPSignal<bool> exportData;
    CDPSignal<bool> debugMode;
    CDPSignal<double> speedCmdA;
    CDPSignal<double> speedCmdB;
    CDPSignal<double> speedCmdC;
    CDPSignal<double> scaledEncSpeed;
    CDPSignal<double> scaledEncPosition;

    CDPTimer timer;
    CDPSignal<double> elapsedTime;

    double encSpeedScaler() { return double(ENC1.speed) / 9.549297; }
    double EncoderRawToDeg_F5888(const EncoderPort& rawEnc) { return static_cast<double>(rawEnc.position) * (360.0 / 65535); }
    void initFC(VaconLib::VaconMarineAppFCPort& FC, bool enable);
    void StopAllMotors();
    MasterSlave ChooseMasterSlave(double errorDeg, double speed);
    double SpeedController(double errorDeg);
    void MoveToPos(double targetDeg, bool antiBacklashEnabled);
    
    double startPos;
    bool gotStartPos = false;
    double degMargin = 10.0;

    double preloadTorque = 0.15 * kMaxTorque; //Nm
    double velocityDeadzone = 0.5; //rad/s

    static constexpr double kLoadTorqueLimit = 2.0; //Nm
    static constexpr double kMaxTorque = 10; //Nm

    void setMasterSlaveTorque(MasterSlave roles, double masterTorque, double slaveTorque);
    void enableLoad(VaconLib::VaconMarineAppFCPort& load, bool enable);
    void enableMasterSlave(VaconLib::VaconMarineAppFCPort& master, VaconLib::VaconMarineAppFCPort& slave, bool enable);


    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;
};

} // namespace AntiBacklashLib

#endif
