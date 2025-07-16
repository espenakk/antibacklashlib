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
    void InitFC(bool FC1Enable, bool FC2Enable, bool FC3Enable);
    void StopAllMotors();
    VaconLib::VaconMarineAppFCPort* ChooseSlave(double errorDeg, double speed);
    double SpeedController(double errorDeg);
    void MoveToPos(double targetDeg, bool antiBacklashEnabled);
    void ApplyPreload(VaconLib::VaconMarineAppFCPort& slave, bool enablePreload);
    
    double startPos;
    bool gotStartPos = false;
    double degMargin = 10.0;
    double loadTorqueLimit = 2.0; //Nm
    double preloadTorque = 0.15 * MAX_TORQUE; // 15% of rated torque(Nm)
    double velocityDeadzone = 0.5; // rad/s
    static constexpr double MAX_TORQUE = 10;


    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;
};

} // namespace AntiBacklashLib

#endif
