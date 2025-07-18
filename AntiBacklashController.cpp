#include "AntiBacklashController.h"
#include <math.h>

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
    FCSpeedRef.Create("FCSpeedRef",this);
    FC1Torque.Create("FC1Torque",this);
    FC2Torque.Create("FC2Torque",this);
    FC3Torque.Create("FC3Torque",this);
    FC1Speed.Create("FC1Speed",this);
    FC2Speed.Create("FC2Speed",this);
    debugMode.Create("debugMode",this);
    speedCmd.Create("speedCmd",this);
    FC1.Create("FC1",this);
    FC2.Create("FC2",this);
    FC3.Create("FC3",this);
    ENC1.Create("ENC1",this);
    loadTorqueLimit.Create("loadTorqueLimit",this);
    maxTorque.Create("maxTorque",this);
    maxSpeed.Create("maxSpeed",this);
    minSpeed.Create("minSpeed",this);
    slowdownRange.Create("slowdownRange",this);
    degMargin.Create("degMargin",this);
    slaveTorqueBase.Create("slaveTorqueBase",this);
    slaveTorqueGain.Create("slaveTorqueGain",this);
    enabled.Create("enabled",this);
    antiBacklashEnabled.Create("antiBacklashEnabled",this);
    timer.Create("timer",this);
    elapsedTime.Create("elapsedTime",this);
    scaledEncPosition.Create("scaledEncPosition",this);
    startAntibacklashTestButton.Create("startAntibacklashTestButton",this);
    scaledEncSpeed.Create("scaledEncSpeed",this);
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
    RegisterStateProcess("Debug",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessDebug,"");
    RegisterStateProcess("PositionTest",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessPositionTest,"");
    RegisterStateProcess("SpeedTimerTest",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessSpeedTimerTest,"");
    RegisterStateTransitionHandler("Null","Debug",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToDebug,"");
    RegisterStateTransitionHandler("Debug","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionDebugToNull,"");
    RegisterStateTransitionHandler("Null","PositionTest",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToPositionTest,"");
    RegisterStateTransitionHandler("PositionTest","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionPositionTestToNull,"");
    RegisterStateTransitionHandler("Null","SpeedTimerTest",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToSpeedTimerTest,"");
    RegisterStateTransitionHandler("SpeedTimerTest","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionSpeedTimerTestToNull,"");
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
    bool isEnabled = false;
    elapsedTime = 0.0;
    initFC(FC1, isEnabled);
    initFC(FC2, isEnabled);
    initFC(FC3, isEnabled);
    if (debugMode) {
        requestedState = "Debug";
    }
    if (startAntibacklashTestButton) {
        requestedState = "Running";
    }
}

void AntiBacklashController::ProcessDebug()
{
    if (!debugMode) {
        requestedState = "Null";
    }
    elapsedTime = 42.0;
    scaledEncSpeed = encSpeedScaler(ENC1);
    scaledEncPosition = encoderRawToDeg_F5888(ENC1);
    scaleFCSpeedTorque(motorRoles);

    enableMasterSlave(motorRoles, enabled);
    enableLoad(motorRoles, enabled);
    double absSpeedCmd = std::abs(speedCmd);
    static double slaveTorque = adaptiveSlaveTorque(absSpeedCmd);
    setMasterSlaveSpeed(motorRoles, absSpeedCmd, absSpeedCmd);
    setMasterSlaveTorque(motorRoles, maxTorque, slaveTorque);

    if (speedCmd > 0) {
        motorRoles.master = &FC1;
        motorRoles.slave = &FC2;
    } else { //Motor B is Master
        motorRoles.master = &FC2;
        motorRoles.slave = &FC1;
    }
}

void AntiBacklashController::ProcessPositionTest()
{
    if (!timer.IsRunning()) {
        timer.Start();
    }
    elapsedTime = timer.TimeElapsed();
    scaledEncSpeed = encSpeedScaler(ENC1);
    scaledEncPosition = encoderRawToDeg_F5888(ENC1);
    scaleFCSpeedTorque(motorRoles);
    enableMasterSlave(motorRoles, true);
    enableLoad(motorRoles, true);
    static int testStep = 0;
    static double targetDeg = 0.0;
    static bool moveStarted = false;
    static double originalMaxSpeed = maxSpeed;
    static double originalDegMargin = degMargin;

    if (!moveStarted) {
        switch (testStep) {
        case 0: targetDeg = scaledEncPosition + 90; break;
        case 1: targetDeg = scaledEncPosition - 270; break;
        case 2: targetDeg = scaledEncPosition + 450; antiBacklashEnabled = true; break;
        case 3: targetDeg = scaledEncPosition - 270; break;
        case 4: targetDeg = scaledEncPosition + 450; antiBacklashEnabled = false; break;
        case 5: targetDeg = scaledEncPosition - 360; maxSpeed = maxSpeed * 2; degMargin = degMargin * 3; break;
        case 6: targetDeg = scaledEncPosition + 540; antiBacklashEnabled = true; break;
        case 7: targetDeg = scaledEncPosition - 360; break;
        case 8: targetDeg = scaledEncPosition + 540; antiBacklashEnabled = false; break;
        case 9: targetDeg = scaledEncPosition - 450; maxSpeed = maxSpeed * 2; break;
        case 10: targetDeg = scaledEncPosition + 630; antiBacklashEnabled = true; break;
        case 11: targetDeg = scaledEncPosition - 450; break;
        case 12: targetDeg = scaledEncPosition + 630; antiBacklashEnabled = false; break;
        case 13: targetDeg = scaledEncPosition - 540; maxSpeed = maxSpeed * 2; break;
        case 14: targetDeg = scaledEncPosition + 720; antiBacklashEnabled = true; break;
        case 15: targetDeg = scaledEncPosition - 540; break;
        case 16: targetDeg = scaledEncPosition + 720; break;

        default:
            stopAllMotors(motorRoles);
            requestedState = "Null";
            antiBacklashEnabled = false;
            testStep = 0;
            moveStarted = false;
            maxSpeed = originalMaxSpeed;
            degMargin = originalDegMargin;
            return;
        }
        moveStarted = true;
    }

    double currentDeg = encoderRawToDeg_F5888(ENC1);
    double errorDeg = targetDeg - currentDeg;
    motorRoles = chooseMasterSlave(errorDeg);
    double speedRef = speedController(errorDeg);
    double slaveTorque = adaptiveSlaveTorque(speedRef);

    setMasterSlaveSpeed(motorRoles, speedRef, speedRef);
    setMasterSlaveTorque(motorRoles, maxTorque, slaveTorque);

    if (abs(errorDeg) < degMargin) {
        testStep++;
        moveStarted = false;
        previousSpeedRef = 0.0;
    } else {
        previousSpeedRef = std::abs(motorRoles.master->SpeedRef);
    }
}



void AntiBacklashController::ProcessSpeedTimerTest()
{
    if (!timer.IsRunning()) {
        timer.Start();
    }
    elapsedTime = timer.TimeElapsed();
    scaledEncSpeed = encSpeedScaler(ENC1);
    scaledEncPosition = encoderRawToDeg_F5888(ENC1);
    scaleFCSpeedTorque(motorRoles);
    enableMasterSlave(motorRoles, true);

}



bool AntiBacklashController::TransitionNullToDebug()
{
    if(requestedState=="Debug") {
        return true;
    }
    else
        return false;
}

bool AntiBacklashController::TransitionDebugToNull()
{
    if(requestedState=="Null") {
        return true;
    }
    else
        return false;
}

bool AntiBacklashController::TransitionNullToPositionTest()
{
    if(requestedState=="Running") {
        return true;
    } else {
        return false;
    }
}

bool AntiBacklashController::TransitionPositionTestToNull()
{
    if(requestedState=="Null") {
        return true;
    }
    else
        return false;
}

bool AntiBacklashController::TransitionNullToSpeedTimerTest()
{
    if(requestedState=="SpeedTimerTest")
        return true;
    else
        return false;
}

bool AntiBacklashController::TransitionSpeedTimerTestToNull()
{
    if(requestedState=="Null")
        return true;
    else
        return false;
}

void AntiBacklashController::initFC(VaconLib::VaconMarineAppFCPort& FC, bool enable) {
    // Power limit at 100%
    FC.PowerLimitGeneratoring = 100;
    FC.PowerLimitMotoring = 100;

    // Closed loop speed control
    FC.ModeSelect = 3;

    FC.Enable = enable;
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
    double deltaSpeedCmd = speedCmd - previousSpeedRef;
    double actualSpeedDiff = abs(speedCmd - encSpeedScaler(ENC1));
    double slaveTorque = 0.0;

    if (deltaSpeedCmd < 0)  {
        double dynamicTorque = slaveTorqueGain * std::max(0.0, -deltaSpeedCmd) * actualSpeedDiff;
        slaveTorque = slaveTorqueBase + dynamicTorque;
    }
    slaveTorque = std::min(slaveTorque, (double)maxTorque);
    if (antiBacklashEnabled) {
        return slaveTorque;
    } else {
        return 0.0;
    }
}

double AntiBacklashController::speedController(double errorDeg) {
    if (abs(errorDeg) <= degMargin)
        return 0.0;
    if (abs(errorDeg) < slowdownRange) {
        return minSpeed + (maxSpeed - minSpeed) * (abs(errorDeg) / slowdownRange);
    }
    return maxSpeed;
}

void AntiBacklashController::setMasterSlaveTorque(MotorRoles& roles, double masterTorque, double slaveTorque) {
    roles.master->TorqueLimitGeneratoring = roles.master->TorqueLimitMotoring = masterTorque;
    roles.slave->TorqueLimitGeneratoring = roles.slave->TorqueLimitMotoring = slaveTorque;
}

void AntiBacklashController::setMasterSlaveSpeed(MotorRoles& roles, double masterSpeed, double slaveSpeed) {
    roles.master->SpeedRef  = masterSpeed;
    roles.slave->SpeedRef = slaveSpeed;
}

void AntiBacklashController::enableMasterSlave(MotorRoles& roles, bool enable) {
    roles.master->Enable = roles.slave->Enable = enable;
}

void AntiBacklashController::enableLoad(MotorRoles& roles, bool enable) {
    if (enable) {
        roles.load->SpeedRef = 0.0;
        roles.load->TorqueLimitMotoring = loadTorqueLimit;
        roles.load->TorqueLimitGeneratoring = loadTorqueLimit;
        roles.load->Enable = true;
    } else {
        roles.load->Enable = false;
        roles.load->TorqueLimitGeneratoring = maxTorque;
        roles.load->TorqueLimitMotoring = maxTorque;
    }
}

void AntiBacklashController::stopAllMotors(MotorRoles& roles) {
    setMasterSlaveTorque(roles, 0.0, 0.0);
    setMasterSlaveSpeed(roles, 0.0, 0.0);
    enableLoad(roles, false);
    enableMasterSlave(roles, false);
}

void AntiBacklashController::scaleFCSpeedTorque(MotorRoles& roles) {
    FC1Speed = FC1.SpeedActual;
    FC2Speed = -FC2.SpeedActual;
    FC1Torque = FC1.TorqueActual;
    FC2Torque = FC2.TorqueActual;
    FC3Torque = -std::abs(FC3.TorqueActual);

    if (roles.master->GetNodeID() == FC1.GetNodeID()) {
        FCSpeedRef = roles.master->SpeedRef;
    } else {
        FCSpeedRef = -roles.master->SpeedRef;
    }
}
