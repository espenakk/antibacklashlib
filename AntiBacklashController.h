#ifndef ANTIBACKLASHLIB_ANTIBACKLASHCONTROLLER_H
#define ANTIBACKLASHLIB_ANTIBACKLASHCONTROLLER_H

#include <SimCmdPort.h>
#include <ABParamPort.h>
#include <CDPAlarm/CDPAlarm.h>
#include <CDPParameter/CDPParameter.h>
#include <CDPSystem/Base/CDPComponent.h>
#include <ShaftPort.h>
#include <Signal/CDPSignal.h>
#include <VaconLib/FCIOPort.h>
#include <VaconLib/VaconMarineAppFCPort.h>
#include <EncoderPort.h>

#include <deque>

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
    void ProcessSpeedCmdOffset();
    void ProcessAdaptiveTorque();
    void ProcessConstTorque();
    void ProcessSlaveDrooping();
    void ProcessActualPositionOffset();
    void ProcessSlaveSpeedRefDelay();
    void ProcessConstrainedSlaveAcceleration();
    bool TransitionNullToSpeedCmdOffset();
    bool TransitionSpeedCmdOffsetToNull();
    bool TransitionNullToAdaptiveTorque();
    bool TransitionAdaptiveTorqueToNull();
    bool TransitionNullToConstTorque();
    bool TransitionConstTorqueToNull();
    bool TransitionNullToSlaveDrooping();
    bool TransitionSlaveDroopingToNull();
    bool TransitionNullToActualPositionOffset();
    bool TransitionActualPositionOffsetToNull();
    bool TransitionNullToSlaveSpeedRefDelay();
    bool TransitionSlaveSpeedRefDelayToNull();
    bool TransitionNullToConstrainedSlaveAcceleration();
    bool TransitionConstrainedSlaveAccelerationToNull();

protected:
    VaconLib::VaconMarineAppFCPort FC1;
    VaconLib::VaconMarineAppFCPort FC2;
    VaconLib::VaconMarineAppFCPort FC3;
    EncoderPort ENC1;
    SimCmdPort SimCmd;
    ShaftPort S1;
    ShaftPort S2;
    ShaftPort S3;
    ABParamPort ABParams;

    CDPSignal<double> SpeedRef;
    CDPSignal<double> ENC1Speed;
    CDPSignal<double> FC1Speed;
    CDPSignal<double> FC2Speed;
    CDPSignal<double> FC1Torque;
    CDPSignal<double> FC2Torque;
    CDPSignal<double> FC3Torque;
    CDPSignal<double> ENC1Position;
    CDPSignal<double> FC1Position;
    CDPSignal<double> FC2Position;
    CDPSignal<double> FC3Position;
    CDPSignal<bool> Running;
    CDPSignal<int> TestIndex;

    struct MotorRoles {
        VaconLib::VaconMarineAppFCPort* master;
        VaconLib::VaconMarineAppFCPort* slave;
        VaconLib::VaconMarineAppFCPort* load;
    };

    MotorRoles motorRoles;

    void chooseMasterSlave(MotorRoles& roles, double speedCmd);
    void scalePlotSignals(MotorRoles& roles);
    void setMasterSlaveTorque(MotorRoles& roles, double masterTorque, double slaveTorque);
    void setMasterSlaveSpeed(MotorRoles& roles, double masterSpeed, double slaveSpeed);
    void setMasterSlaveDroop(MotorRoles& roles, double masterDroop, double slaveDroop);
    void enableLoad(MotorRoles& roles, bool enable);
    void enableMasterSlave(MotorRoles& roles, bool enable);
    void stopAllMotors(MotorRoles& roles);

    bool startTestTransition(const std::string& targetState);
    bool stopTestTransition(const std::string& targetState);

    double adaptiveSlaveTorque(double& speedCmd);
    double encSpeedScaler(const EncoderPort& enc) { return double(enc.speed) / 9.549297; }
    double encoderRawToDeg_F5888(const EncoderPort& enc) { return double(enc.position) * (360.0 / 65535); }
    double fcShaftRoundsAngleToDeg(const ShaftPort& s) { return double(s.ShaftRounds) * (360) + double(s.ShaftAngle); }

    double encStartAngle;
    double FC1StartAngle;
    double FC2StartAngle;
    double FC3StartAngle;
    double lastFC1Position;
    double lastFC2Position;
    double lastFC3Position;

    static double constexpr slaveDelay{0.150};
    std::deque<double> speedHistory;


    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;
};

} // namespace AntiBacklashLib

#endif
