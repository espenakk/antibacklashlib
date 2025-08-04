#ifndef ANTIBACKLASHLIB_ANTIBACKLASHCONTROLLER_H
#define ANTIBACKLASHLIB_ANTIBACKLASHCONTROLLER_H

#include <SimCmdPort.h>
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
    void ProcessSpeedCmdOffset();
    void ProcessDecelTorque();
    void ProcessConstTorque();
    bool TransitionNullToSpeedCmdOffset();
    bool TransitionSpeedCmdOffsetToNull();
    bool TransitionNullToDecelTorque();
    bool TransitionDecelTorqueToNull();
    bool TransitionNullToConstTorque();
    bool TransitionConstTorqueToNull();

protected:
    VaconLib::VaconMarineAppFCPort FC1;
    VaconLib::VaconMarineAppFCPort FC2;
    VaconLib::VaconMarineAppFCPort FC3;
    EncoderPort ENC1;
    SimCmdPort SimCmd;
    CDPParameter Offset;
    CDPParameter SlaveTorqueBase;
    CDPParameter SlaveTorqueGain;
    CDPParameter LoadTorqueLimit;
    CDPParameter MaxTorque;
    CDPParameter SlaveDroop;
    CDPParameter MasterDroop;

    
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
    CDPSignal<int> AntiBacklashMode;

    double previousSpeedRef;
    MotorRoles motorRoles;

    MotorRoles chooseMasterSlave(double error);
    void scalePlotSignals(MotorRoles& roles);
    void initFCs(MotorRoles& roles);
    void setMasterSlaveTorque(MotorRoles& roles, double masterTorque, double slaveTorque);
    void setMasterSlaveSpeed(MotorRoles& roles, double masterSpeed, double slaveSpeed);
    void setMasterSlaveDroop(MotorRoles& roles, double masterDroop, double slaveDroop);
    void enableLoad(MotorRoles& roles, bool enable);
    void enableMasterSlave(MotorRoles& roles, bool enable);
    void stopAllMotors(MotorRoles& roles);

    double adaptiveSlaveTorque(double& speedCmd);

    double encSpeedScaler(const EncoderPort& enc) { return double(enc.speed) / 9.549297; }
    double encoderRawToDeg_F5888(const EncoderPort& enc) { return double(enc.position) * (360.0 / 65535); }
    double fcShaftRoundsAngleToDeg(const VaconLib::VaconMarineAppFCPort& fc) { return double(fc.ShaftRounds) * (360) + (double(fc.ShaftAngle)) * (180 / 3.14159265358979323846); } // Maybe ShaftAngle is in degrees already?

    using CDPComponent::fs;
    using CDPComponent::requestedState;
    using CDPComponent::ts;
};

} // namespace AntiBacklashLib

#endif
