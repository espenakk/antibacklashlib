#include "AntiBacklashController.h"

using namespace AntiBacklashLib;
/*!
  \class AntiBacklashLib::AntiBacklashController
  \inmodule AntiBacklashLib

  \section1 Usage

  Add documentation for AntiBacklashController here.
*/

/*!
  \internal
  \brief Component constructor. The first function to be called. Can be used to initialize member variables, etc.
*/
AntiBacklashController::AntiBacklashController(): previousSpeedRef(0.0)
{
    motorRoles.master = &FC1;
    motorRoles.slave = &FC2;
    motorRoles.load = &FC3;
}

/*!
  \internal
  \brief Component destructor. The last function to be called. Typically used to clean up when stopping, like freeing
  allocated memory, etc.
*/
AntiBacklashController::~AntiBacklashController()
{
}

/*!
  \internal
  \brief Creates the component instance and other related CDP objects. Called after constructor

  Note, that this function is normally filled automatically by the code generator.
*/
void AntiBacklashController::Create(const char* fullName)
{
    CDPComponent::Create(fullName);
    FC1Speed.Create("FC1Speed",this);
    FC2Speed.Create("FC2Speed",this);
    FC1Torque.Create("FC1Torque",this);
    FC2Torque.Create("FC2Torque",this);
    FC3Torque.Create("FC3Torque",this);
    ENC1Position.Create("ENC1Position",this);
    Running.Create("Running",this);
    TestIndex.Create("TestIndex",this);
    SpeedRef.Create("SpeedRef",this);
    ENC1Speed.Create("ENC1Speed",this);
    AntiBacklashMode.Create("AntiBacklashMode",this);
    FC1.Create("FC1",this);
    FC2.Create("FC2",this);
    FC3.Create("FC3",this);
    ENC1.Create("ENC1",this);
    SimCmd.Create("SimCmd",this);
    LoadTorqueLimit.Create("LoadTorqueLimit",this);
    MaxTorque.Create("MaxTorque",this);
    SlaveTorqueBase.Create("SlaveTorqueBase",this);
    SlaveTorqueGain.Create("SlaveTorqueGain",this);
    Offset.Create("Offset",this);
    SlaveDroop.Create("SlaveDroop",this);
    MasterDroop.Create("MasterDroop",this);
}

/*!
  \internal
  \brief Creates a model instance for this class and fills model data. Registers messages, states and state transitions.

  Note, that this function is normally filled automatically by the code generator.
*/
void AntiBacklashController::CreateModel()
{
    CDPComponent::CreateModel();

    RegisterStateProcess("Null", (CDPCOMPONENT_STATEPROCESS) &AntiBacklashController::ProcessNull, "Initial Null state");
    RegisterStateProcess("SpeedCmdOffset",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessSpeedCmdOffset,"");
    RegisterStateProcess("DecelTorque",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessDecelTorque,"");
    RegisterStateProcess("ConstTorque",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessConstTorque,"");
    RegisterStateTransitionHandler("Null","SpeedCmdOffset",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToSpeedCmdOffset,"");
    RegisterStateTransitionHandler("SpeedCmdOffset","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionSpeedCmdOffsetToNull,"");
    RegisterStateTransitionHandler("Null","DecelTorque",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToDecelTorque,"");
    RegisterStateTransitionHandler("DecelTorque","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionDecelTorqueToNull,"");
    RegisterStateTransitionHandler("Null","ConstTorque",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToConstTorque,"");
    RegisterStateTransitionHandler("ConstTorque","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionConstTorqueToNull,"");
}

/*!
  \internal
  \brief Configures the component by reading the configuration. Called after Create()/CreateModel().

  Note, that this function is normally filled automatically by the code generator.
*/
void AntiBacklashController::Configure(const char* componentXML)
{
    CDPComponent::Configure(componentXML);
}

/*!
 \brief Component Null state processing function

 Write your component's processing code here. When the component is simple, all the processing code may reside here.
 This function gets called periodically. The period is changed by setting the "fs" (frequency) Property when you use
 the component in a project.
 Functions called ProcessSTATENAME(), like ProcessNull(), are state processing functions and are only called when
 components are in given state. The default component state is "Null".
 Note, that state processing functions are not allowed to block (i.e. read files, sleep, communicate with network in
 blocking mode etc.) For blocking code use the 'Threaded Component Model' wizard instead.

 Please consult CDP Studio "Code Mode Manual" for more information and examples.
*/
void AntiBacklashController::ProcessNull()
{
    initFCs(motorRoles);
    scalePlotSignals(motorRoles);

    if (SimCmd.EnableFCs) {
        switch (AntiBacklashMode) {
        case 0:
            requestedState = "DecelTorque";
            break;
        case 1:
            requestedState = "ConstTorque";
            break;
        case 2:
            requestedState = "SpeedCmdOffset";
            break;
        default:
            requestedState = "Null";
            break;
        }
    }
}

void AntiBacklashController::ProcessDecelTorque()
{
    if (!SimCmd.EnableFCs) {
        requestedState = "Null";
    }
    scalePlotSignals(motorRoles);

    motorRoles = chooseMasterSlave(SimCmd.SpeedCMD);
    double absSpeed = std::abs(SimCmd.SpeedCMD);
    double masterSpeed = absSpeed;
    double slaveSpeed = absSpeed;
    double masterTorque = MaxTorque;
    double slaveTorque = adaptiveSlaveTorque(absSpeed);
    double masterDroop;
    double slaveDroop;

    if (SimCmd.AntiBacklashEnabled) {
        masterDroop = MasterDroop;
        slaveDroop = SlaveDroop;
    } else {
        masterDroop = 0.0;
        slaveDroop = 0.0;
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, masterTorque, slaveTorque);
    setMasterSlaveDroop(motorRoles, masterDroop, slaveDroop);
    previousSpeedRef = absSpeed;
}



void AntiBacklashController::ProcessConstTorque()
{
    if (!SimCmd.EnableFCs) {
        requestedState = "Null";
    }
    scalePlotSignals(motorRoles);

    motorRoles = chooseMasterSlave(SimCmd.SpeedCMD);
    double absSpeed = std::abs(SimCmd.SpeedCMD);
    double masterSpeed = absSpeed;
    double slaveSpeed = absSpeed;
    double masterTorque = MaxTorque;
    double slaveTorque;
    double masterDroop;
    double slaveDroop;

    if (SimCmd.AntiBacklashEnabled) {
        masterDroop = MasterDroop;
        slaveDroop = SlaveDroop;
        slaveTorque = SlaveTorqueBase;
    } else {
        masterDroop = 0.0;
        slaveDroop = 0.0;
        slaveTorque = 0;
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, masterTorque, slaveTorque);
    setMasterSlaveDroop(motorRoles, masterDroop, slaveDroop);
}

void AntiBacklashController::ProcessSpeedCmdOffset()
{
    if (!SimCmd.EnableFCs) {
        requestedState = "Null";
    }
    scalePlotSignals(motorRoles);

    motorRoles = chooseMasterSlave(SimCmd.SpeedCMD);
    double absSpeed = std::abs(SimCmd.SpeedCMD);
    double masterSpeed = absSpeed;
    double slaveSpeed = -absSpeed;
    double masterTorque = MaxTorque;
    double slaveTorque = MaxTorque;
    double masterDroop;
    double slaveDroop;

    if (SimCmd.AntiBacklashEnabled) {
        masterDroop = MasterDroop;
        slaveDroop = SlaveDroop;
        masterSpeed *= (1 + (Offset / 100));
        slaveSpeed *= (1 - (Offset / 100));
    } else {
        masterDroop = 0.0;
        slaveDroop = 0.0;
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, masterTorque, slaveTorque);
    setMasterSlaveDroop(motorRoles, masterDroop, slaveDroop);
}

bool AntiBacklashController::TransitionNullToDecelTorque()
{
    if(requestedState=="DecelTorque") {
        enableMasterSlave(motorRoles, true);
        enableLoad(motorRoles, true);
        TestIndex = TestIndex + 1;
        Running = true;
        return true;
    }
    return false;
}

bool AntiBacklashController::TransitionDecelTorqueToNull()
{
    if(requestedState=="Null") {
        stopAllMotors(motorRoles);
        Running = false;
        return true;
    }
    return false;
}

bool AntiBacklashController::TransitionNullToConstTorque()
{
    if(requestedState=="ConstTorque") {
        enableMasterSlave(motorRoles, true);
        enableLoad(motorRoles, true);
        TestIndex = TestIndex + 1;
        Running = true;
        return true;
    }
    return false;
}

bool AntiBacklashController::TransitionConstTorqueToNull()
{
    if(requestedState=="Null") {
        stopAllMotors(motorRoles);
        Running = false;
        return true;
    }
    return false;
}

bool AntiBacklashController::TransitionNullToSpeedCmdOffset()
{
    if(requestedState=="SpeedCmdOffset") {
        enableMasterSlave(motorRoles, true);
        enableLoad(motorRoles, true);
        TestIndex = TestIndex + 1;
        Running = true;
        return true;
    }
    return false;
}

bool AntiBacklashController::TransitionSpeedCmdOffsetToNull()
{
    if(requestedState=="Null") {
        stopAllMotors(motorRoles);
        Running = false;
        return true;
    }
    return false;
}

void AntiBacklashController::initFCs(MotorRoles& roles) {
    // Power limit at 100%
    roles.master->PowerLimitGeneratoring = 100;
    roles.master->PowerLimitMotoring = 100;
    roles.slave->PowerLimitGeneratoring = 100;
    roles.slave->PowerLimitMotoring = 100;
    roles.load->PowerLimitGeneratoring = 100;
    roles.load->PowerLimitMotoring = 100;

    // Closed loop speed control
    roles.master->ModeSelect = 3;
    roles.slave->ModeSelect = 3;
    roles.load->ModeSelect = 3;

    roles.master->Enable = false;
    roles.slave->Enable = false;
    roles.load->Enable = false;
}

MotorRoles AntiBacklashController::chooseMasterSlave(double error) {
    MotorRoles roles;
    if (error > 0) {
        roles.master = &FC1;
        roles.slave = &FC2;
        roles.load = &FC3;
        return roles;
    } else {
        roles.master = &FC2;
        roles.slave = &FC1;
        roles.load = &FC3;
        return roles;
    }
}

double AntiBacklashController::adaptiveSlaveTorque(double& speedCmd) {
    double acceleration = (speedCmd - previousSpeedRef) / ts;
    if (acceleration < 0.0)  {
        double dynamicTorque = SlaveTorqueGain * std::max(0.0, -acceleration);
        return SlaveTorqueBase + dynamicTorque;
    }
    return 0.0;
}

void AntiBacklashController::setMasterSlaveTorque(MotorRoles& roles, double masterTorque, double slaveTorque) {
    roles.master->TorqueLimitGeneratoring = roles.master->TorqueLimitMotoring = masterTorque;
    roles.slave->TorqueLimitGeneratoring = roles.slave->TorqueLimitMotoring = slaveTorque;
}

void AntiBacklashController::setMasterSlaveSpeed(MotorRoles& roles, double masterSpeed, double slaveSpeed) {
    roles.master->SpeedRef  = masterSpeed;
    roles.slave->SpeedRef = slaveSpeed;
}

void AntiBacklashController::setMasterSlaveDroop(MotorRoles& roles, double masterDroop, double slaveDroop) {
    roles.slave->LoadDrooping = masterDroop;
    roles.master->LoadDrooping = slaveDroop;
}

void AntiBacklashController::enableMasterSlave(MotorRoles& roles, bool enable) {
    roles.master->Enable = roles.slave->Enable = enable;
}

void AntiBacklashController::enableLoad(MotorRoles& roles, bool enable) {
    if (enable) {
        roles.load->SpeedRef = 0.0;
        roles.load->TorqueLimitMotoring = LoadTorqueLimit;
        roles.load->TorqueLimitGeneratoring = LoadTorqueLimit;
        roles.load->Enable = true;
    } else {
        roles.load->Enable = false;
        roles.load->TorqueLimitGeneratoring = MaxTorque;
        roles.load->TorqueLimitMotoring = MaxTorque;
    }
}

void AntiBacklashController::stopAllMotors(MotorRoles& roles) {
    setMasterSlaveTorque(roles, 0.0, 0.0);
    setMasterSlaveSpeed(roles, 0.0, 0.0);
    enableLoad(roles, false);
    enableMasterSlave(roles, false);
}

void AntiBacklashController::scalePlotSignals(MotorRoles& roles) {
    FC1Speed = FC1.SpeedActual;
    FC2Speed = -FC2.SpeedActual;
    FC1Torque = FC1.TorqueActual;
    FC2Torque = FC2.TorqueActual;
    FC3Torque = -std::abs(FC3.TorqueActual);

    ENC1Speed = -encSpeedScaler(ENC1);
    ENC1Position = encoderRawToDeg_F5888(ENC1);

    if (roles.master->GetNodeID() == FC1.GetNodeID()) {
        SpeedRef = roles.master->SpeedRef;
    } else {
        SpeedRef = -roles.master->SpeedRef;
    }
}
