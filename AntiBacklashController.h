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

    void antibacklashTestScript(double t, int speedNoLoad, int speedLoad);

    void InitFC(bool FC1Enable, bool FC2Enable, bool FC3Enable);
    double EncoderRawToDeg_F5888(const EncoderPort& rawEnc);
    void StopAll();
    VaconLib::VaconMarineAppFCPort* ChooseDir(double errorDeg, double speed);
    void MoveToPos(double targetDeg, bool antiBacklashEnabled, bool loadEnabled);
    double SpeedController(double errorDeg);
    double PIController(double errorDeg);
    void ApplyPreload(VaconLib::VaconMarineAppFCPort& slave, bool enablePreload);

    void TestScriptPos();
    void EncoderTest();

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
    CDPSignal<int> scaledEncSpeed;

    CDPTimer timer;
    CDPSignal<double> elapsedTime;

    int encSpeedScaler();
    
    double startPos;
    bool gotStartPos = false;
    double degMargin = 10.0;
    CDPTimer PITimer;
    double loadTorLim = 0.5; //Nm

    // antiBacklash
    double preloadTorque = 0.05 * 4; // 5% of rated torque(Nm)
    double velocityDeadzone = 0.5; // rad/s


    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;
};

} // namespace AntiBacklashLib

#endif
