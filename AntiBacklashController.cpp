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
AntiBacklashController::AntiBacklashController()
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
    FC1Position.Create("FC1Position",this);
    FC2Position.Create("FC2Position",this);
    FC3Position.Create("FC3Position",this);
    Running.Create("Running",this);
    TestIndex.Create("TestIndex",this);
    SpeedRef.Create("SpeedRef",this);
    ENC1Speed.Create("ENC1Speed",this);
    FC1.Create("FC1",this);
    FC2.Create("FC2",this);
    FC3.Create("FC3",this);
    ENC1.Create("ENC1",this);
    SimCmd.Create("SimCmd",this);
    S1.Create("S1",this);
    S2.Create("S2",this);
    S3.Create("S3",this);
    ABParams.Create("ABParams",this);
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
    RegisterStateProcess("AdaptiveTorque",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessAdaptiveTorque,"");
    RegisterStateProcess("ConstTorque",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessConstTorque,"");
    RegisterStateProcess("SlaveDrooping",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessSlaveDrooping,"");
    RegisterStateProcess("ActualPositionOffset",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessActualPositionOffset,"");
    RegisterStateProcess("SlaveSpeedRefDelay",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessSlaveSpeedRefDelay,"");
    RegisterStateProcess("ConstrainedSlaveAcceleration",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessConstrainedSlaveAcceleration,"");
    RegisterStateTransitionHandler("Null","SpeedCmdOffset",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToSpeedCmdOffset,"");
    RegisterStateTransitionHandler("SpeedCmdOffset","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionSpeedCmdOffsetToNull,"");
    RegisterStateTransitionHandler("Null","AdaptiveTorque",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToAdaptiveTorque,"");
    RegisterStateTransitionHandler("AdaptiveTorque","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionAdaptiveTorqueToNull,"");
    RegisterStateTransitionHandler("Null","ConstTorque",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToConstTorque,"");
    RegisterStateTransitionHandler("ConstTorque","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionConstTorqueToNull,"");
    RegisterStateTransitionHandler("Null","SlaveDrooping",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToSlaveDrooping,"");
    RegisterStateTransitionHandler("SlaveDrooping","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionSlaveDroopingToNull,"");
    RegisterStateTransitionHandler("Null","ActualPositionOffset",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToActualPositionOffset,"");
    RegisterStateTransitionHandler("ActualPositionOffset","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionActualPositionOffsetToNull,"");
    RegisterStateTransitionHandler("Null","SlaveSpeedRefDelay",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToSlaveSpeedRefDelay,"");
    RegisterStateTransitionHandler("SlaveSpeedRefDelay","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionSlaveSpeedRefDelayToNull,"");
    RegisterStateTransitionHandler("Null","ConstrainedSlaveAcceleration",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToConstrainedSlaveAcceleration,"");
    RegisterStateTransitionHandler("ConstrainedSlaveAcceleration","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionConstrainedSlaveAccelerationToNull,"");
}

/*!
  \internal
  \brief Configures the component by reading the configuration. Called after Create()/CreateModel().

  Note, that this function is normally filled automatically by the code generator.t
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
    scalePlotSignals(motorRoles);

    if (SimCmd.EnableFCs) {
        switch (ABParams.AntiBacklashMode) {
        case 0:
            requestedState = "AdaptiveTorque";
            break;
        case 1:
            requestedState = "ConstTorque";
            break;
        case 2:
            requestedState = "SpeedCmdOffset";
            break;
        case 3:
            requestedState = "SlaveDrooping";
            break;
        case 4:
            requestedState = "ActualPositionOffset";
            break;
        case 5:
            requestedState = "SlaveSpeedRefDelay";
            break;
        case 6:
            requestedState = "ConstrainedSlaveAcceleration";
            break;
        default:
            requestedState = "Null";
            break;
        }
    }
}

void AntiBacklashController::ProcessAdaptiveTorque()
{
    if (!SimCmd.EnableFCs) {
        requestedState = "Null";
    }
    scalePlotSignals(motorRoles);
    chooseMasterSlave(motorRoles, SimCmd.SpeedCMD);
    double masterSpeed = std::abs(SimCmd.SpeedCMD);
    double slaveSpeed = -masterSpeed;
    double masterTorque = ABParams.MaxTorque;
    double slaveTorque = ABParams.MaxTorque;
    double masterDroop = ABParams.MasterDroop;
    double slaveDroop = ABParams.SlaveDroop;

    if (SimCmd.AntiBacklashEnabled) {
        slaveTorque = adaptiveSlaveTorque(masterSpeed);
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, masterTorque, slaveTorque);
    setMasterSlaveDroop(motorRoles, masterDroop, slaveDroop);
}

void AntiBacklashController::ProcessConstTorque()
{
    if (!SimCmd.EnableFCs) {
        requestedState = "Null";
    }
    scalePlotSignals(motorRoles);
    chooseMasterSlave(motorRoles, SimCmd.SpeedCMD);
    double masterSpeed = std::abs(SimCmd.SpeedCMD);
    double slaveSpeed = -masterSpeed;
    double masterTorque = ABParams.MaxTorque;
    double slaveTorque = ABParams.MaxTorque;
    double masterDroop = ABParams.MasterDroop;
    double slaveDroop = ABParams.SlaveDroop;

    if (SimCmd.AntiBacklashEnabled) {
        slaveTorque = ABParams.SlaveTorqueBase;
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
    chooseMasterSlave(motorRoles, SimCmd.SpeedCMD);
    double masterSpeed = std::abs(SimCmd.SpeedCMD);
    double slaveSpeed = -masterSpeed;
    double masterTorque = ABParams.MaxTorque;
    double slaveTorque = ABParams.MaxTorque;
    double masterDroop = ABParams.MasterDroop;
    double slaveDroop = ABParams.SlaveDroop;

    if (SimCmd.AntiBacklashEnabled) {
        masterSpeed *= (1 + (ABParams.Offset / 100));
        slaveSpeed *= (1 - (ABParams.Offset / 100));
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, masterTorque, slaveTorque);
    setMasterSlaveDroop(motorRoles, masterDroop, slaveDroop);
}

void AntiBacklashController::ProcessSlaveDrooping()
{
    if (!SimCmd.EnableFCs) {
        requestedState = "Null";
    }
    scalePlotSignals(motorRoles);
    chooseMasterSlave(motorRoles, SimCmd.SpeedCMD);
    double masterSpeed = std::abs(SimCmd.SpeedCMD);
    double slaveSpeed = -masterSpeed;
    double masterTorque = ABParams.MaxTorque;
    double slaveTorque = ABParams.MaxTorque;
    double masterDroop = 0.0;
    double slaveDroop = 0.0;

    if (SimCmd.AntiBacklashEnabled) {
        masterDroop = ABParams.MasterDroop;
        slaveDroop = ABParams.SlaveDroop;
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, masterTorque, slaveTorque);
    setMasterSlaveDroop(motorRoles, masterDroop, slaveDroop);
}


void AntiBacklashController::ProcessActualPositionOffset()
{
    if (!SimCmd.EnableFCs) {
        requestedState = "Null";
    }
    scalePlotSignals(motorRoles);
    chooseMasterSlave(motorRoles, SimCmd.SpeedCMD);
    double masterSpeed = std::abs(SimCmd.SpeedCMD);
    double slaveSpeed = -masterSpeed;
    double masterTorque = ABParams.MaxTorque;
    double slaveTorque = ABParams.MaxTorque;
    double masterDroop = ABParams.MasterDroop;
    double slaveDroop = ABParams.SlaveDroop;
    static double masterPosition = 0;
    static double slavePosition = 0;

    if (motorRoles.master->GetNodeID() == FC1.GetNodeID()) {
        masterPosition = double(FC1Position);
        slavePosition = double(FC2Position);
    }
    if (motorRoles.master->GetNodeID() == FC2.GetNodeID()) {
        masterPosition = double(FC2Position);
        slavePosition = double(FC1Position);
    }

    double setPoint = 180;
    double error = (masterPosition - setPoint) - slavePosition;
    double kp = 0.001;
    double pOutput = -kp * (error);

    if (SimCmd.AntiBacklashEnabled) {
        slaveSpeed += pOutput;
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, masterTorque, slaveTorque);
    setMasterSlaveDroop(motorRoles, masterDroop, slaveDroop);
}


void AntiBacklashController::ProcessSlaveSpeedRefDelay()
{
    if (!SimCmd.EnableFCs) {
        requestedState = "Null";
    }
    scalePlotSignals(motorRoles);
    chooseMasterSlave(motorRoles, SimCmd.SpeedCMD);
    double masterSpeed = std::abs(SimCmd.SpeedCMD);
    double slaveSpeed = -masterSpeed;
    double masterTorque = ABParams.MaxTorque;
    double slaveTorque = ABParams.MaxTorque;
    double masterDroop = ABParams.MasterDroop;
    double slaveDroop = ABParams.MasterDroop;

    if (SimCmd.AntiBacklashEnabled) {
        speedHistory.push_back(masterSpeed);
        unsigned int delayInSamples = static_cast<unsigned int>(slaveDelay) / (ts * 1000.0);
        double delayedSpeed = 0.0;
        if (speedHistory.size() > delayInSamples) {
            delayedSpeed = speedHistory.front();
            speedHistory.pop_front();
        }
        slaveSpeed = -delayedSpeed;
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, masterTorque, slaveTorque);
    setMasterSlaveDroop(motorRoles, masterDroop, slaveDroop);
}


// Not done
void AntiBacklashController::ProcessConstrainedSlaveAcceleration()
{
    if (!SimCmd.EnableFCs) {
        requestedState = "Null";
    }
    scalePlotSignals(motorRoles);
    chooseMasterSlave(motorRoles, SimCmd.SpeedCMD);
    double masterSpeed = std::abs(SimCmd.SpeedCMD);
    double slaveSpeed = -masterSpeed;
    double masterTorque = ABParams.MaxTorque;
    double slaveTorque = ABParams.MaxTorque;
    double masterDroop = ABParams.MasterDroop;
    double slaveDroop = ABParams.MasterDroop;

    if (SimCmd.AntiBacklashEnabled) {
        // Implement
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, masterTorque, slaveTorque);
    setMasterSlaveDroop(motorRoles, masterDroop, slaveDroop);
}

// Transition Null -> State

bool AntiBacklashController::TransitionNullToAdaptiveTorque()
{
    return startTestTransition("AdaptiveTorque");
}

bool AntiBacklashController::TransitionNullToConstTorque()
{
    return startTestTransition("ConstTorque");
}

bool AntiBacklashController::TransitionNullToSpeedCmdOffset()
{
    return startTestTransition("SpeedCmdOffset");
}

bool AntiBacklashController::TransitionNullToSlaveDrooping()
{
    return startTestTransition("SlaveDrooping");
}

bool AntiBacklashController::TransitionNullToActualPositionOffset()
{
    return startTestTransition("ActualPositionOffset");
}

bool AntiBacklashController::TransitionNullToSlaveSpeedRefDelay()
{
    return startTestTransition("SlaveSpeedRefDelay");
}

bool AntiBacklashController::TransitionNullToConstrainedSlaveAcceleration()
{
    return startTestTransition("ConstrainedSlaveAcceleration");
}

// Transition State -> Null

bool AntiBacklashController::TransitionAdaptiveTorqueToNull()
{
    return stopTestTransition("AdaptiveTorque");
}

bool AntiBacklashController::TransitionConstTorqueToNull()
{
    return stopTestTransition("ConstantTorque");
}

bool AntiBacklashController::TransitionSpeedCmdOffsetToNull()
{
    return stopTestTransition("SpeedCmdOffset");
}

bool AntiBacklashController::TransitionSlaveDroopingToNull()
{
    return stopTestTransition("SlaveDrooping");
}

bool AntiBacklashController::TransitionActualPositionOffsetToNull()
{
    return stopTestTransition("ActualPositionOffset");
}

bool AntiBacklashController::TransitionSlaveSpeedRefDelayToNull()
{
    return stopTestTransition("SlaveSpeedRefDelay");
}

bool AntiBacklashController::TransitionConstrainedSlaveAccelerationToNull()
{
    return stopTestTransition("ConstrainedSlaveAcceleration");
}

bool AntiBacklashController::startTestTransition(const std::string& targetState)
{
    if (requestedState == targetState) {
        enableMasterSlave(motorRoles, true);
        enableLoad(motorRoles, true);
        TestIndex = TestIndex + 1;
        Running = true;

        // Reset starting angles
        encStartAngle = encoderRawToDeg_F5888(ENC1);
        FC1StartAngle = fcShaftRoundsAngleToDeg(S1);
        FC2StartAngle = fcShaftRoundsAngleToDeg(S2);
        FC3StartAngle = fcShaftRoundsAngleToDeg(S3);
        lastFC1Position = 0.0;
        lastFC2Position = 0.0;
        lastFC3Position = 0.0;

        speedHistory.clear();

        CDPMessage((targetState + " test started.\n").c_str());
        return true;
    }
    return false;
}

bool AntiBacklashController::stopTestTransition(const std::string& targetState)
{
    if (requestedState == "Null") {
        stopAllMotors(motorRoles);
        Running = false;
        CDPMessage((targetState + " test finished.\n").c_str());
        return true;
    }
    return false;
}

void AntiBacklashController::chooseMasterSlave(MotorRoles& roles, double speedCmd) {
    if (speedCmd > 0) {
        roles.master = &FC1;
        roles.slave = &FC2;
    }
    if (speedCmd < 0) {
        roles.master = &FC2;
        roles.slave = &FC1;
    }
}

double AntiBacklashController::adaptiveSlaveTorque(double& speedCmd) {
    double dynamicTorque = speedCmd * (1 / ABParams.SlaveTorqueGain);
    return ABParams.SlaveTorqueBase + dynamicTorque;
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

void AntiBacklashController::enableMasterSlave(MotorRoles& roles, bool enable = true) {
    if (enable) {
        roles.master->PowerLimitGeneratoring = roles.slave->PowerLimitGeneratoring = 100;
        roles.master->PowerLimitMotoring = roles.slave->PowerLimitMotoring = 100;
        roles.master->ModeSelect = 3;
        roles.slave->ModeSelect = 3;
        roles.master->Enable = roles.slave->Enable = true;
    } else {
        roles.master->Enable = roles.slave->Enable = false;
        roles.master->ModeSelect = 3;
        roles.slave->ModeSelect = 3;
        roles.master->PowerLimitGeneratoring = roles.slave->PowerLimitGeneratoring = 0;
        roles.master->PowerLimitMotoring = roles.slave->PowerLimitMotoring = 0;
    }

}

void AntiBacklashController::enableLoad(MotorRoles& roles, bool enable = true) {
    if (enable) {
        roles.load->SpeedRef = 0.0;
        roles.load->ModeSelect = 3;
        roles.load->TorqueLimitMotoring = roles.load->TorqueLimitGeneratoring = ABParams.LoadTorqueLimit;
        roles.load->PowerLimitGeneratoring = roles.load->PowerLimitMotoring = 100;
        roles.load->Enable = true;
    } else {
        roles.load->Enable = false;
        roles.load->ModeSelect = 3;
        roles.load->TorqueLimitGeneratoring = roles.load->TorqueLimitMotoring = 0;
        roles.load->PowerLimitGeneratoring = roles.load->PowerLimitMotoring = 0;
    }
}

void AntiBacklashController::stopAllMotors(MotorRoles& roles) {
    setMasterSlaveTorque(roles, 0.0, 0.0);
    setMasterSlaveSpeed(roles, 0.0, 0.0);
    enableLoad(roles, false);
    enableMasterSlave(roles, false);
}

void AntiBacklashController::scalePlotSignals(MotorRoles& roles) { //This function is a little bit cursed, please fix :)
    FC1Speed = FC1.SpeedActual;
    FC2Speed = -FC2.SpeedActual;
    FC1Torque = FC1.TorqueActual;
    FC2Torque = FC2.TorqueActual;
    FC3Torque = -std::abs(FC3.TorqueActual);
    FC1Position = fcShaftRoundsAngleToDeg(S1) - FC1StartAngle;
    FC2Position = -(fcShaftRoundsAngleToDeg(S2) - FC2StartAngle);
    FC3Position = fcShaftRoundsAngleToDeg(S3) - FC3StartAngle;

    if (std::abs(FC1Position - lastFC1Position) < 300) {
        lastFC1Position = FC1Position;
    } else {
        FC1Position = lastFC1Position;
    }
    if (std::abs(FC2Position - lastFC2Position) < 300) {
        lastFC2Position = FC2Position;
    } else {
        FC2Position = lastFC2Position;
    }
    if (std::abs(FC3Position - lastFC3Position) < 300) {
        lastFC3Position = FC3Position;
    } else {
        FC3Position = lastFC3Position;
    }

    ENC1Speed = -encSpeedScaler(ENC1);
    ENC1Position = encoderRawToDeg_F5888(ENC1) - encStartAngle;

    if (roles.master->GetNodeID() == FC1.GetNodeID()) {
        SpeedRef = roles.master->SpeedRef;
    }
    if (roles.master->GetNodeID() == FC2.GetNodeID()) {
        SpeedRef = -roles.master->SpeedRef;
    }
}
