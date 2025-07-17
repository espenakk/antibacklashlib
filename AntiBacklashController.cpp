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
    debugMode.Create("debugMode",this);
    speedCmdA.Create("speedCmdA",this);
    speedCmdB.Create("speedCmdB",this);
    speedCmdC.Create("speedCmdC",this);
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
    slaveTorque.Create("slaveTorque",this);
    masterDroop.Create("masterDroop",this);
    slaveDroop.Create("slaveDroop",this);
    loadDroop.Create("loadDroop",this);
    enabled.Create("enabled",this);
    loadEnabled.Create("loadEnabled",this);
    dir.Create("dir",this);
    antiBacklashEnabled.Create("antiBacklashEnabled",this);
    dirC.Create("dirC",this);
    timer.Create("timer",this);
    elapsedTime.Create("elapsedTime",this);
    scaledEncPosition.Create("scaledEncPosition",this);
    startAntibacklashTestButton.Create("startAntibacklashTestButton",this);
    runningAntiBacklashTestScript.Create("runningAntiBacklashTestScript",this);
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
    RegisterStateProcess("Running",(CDPCOMPONENT_STATEPROCESS)&AntiBacklashController::ProcessRunning,"");
    RegisterStateTransitionHandler("Null","Debug",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToDebug,"");
    RegisterStateTransitionHandler("Debug","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionDebugToNull,"");
    RegisterStateTransitionHandler("Null","Running",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionNullToRunning,"");
    RegisterStateTransitionHandler("Running","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&AntiBacklashController::TransitionRunningToNull,"");
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
    setLoadDrooping(motorRoles, masterDroop, slaveDroop, loadDroop);
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
    scaledEncPosition = EncoderRawToDeg_F5888(ENC1);

    MotorRoles motorRoles;

    enableMasterSlave(motorRoles, enabled);
    enableLoad(motorRoles, loadEnabled);

    if (dir) { //Motor A is Master
        motorRoles.master = &FC1;
        motorRoles.slave = &FC2;
        setMasterSlaveTorque(motorRoles, maxTorque, maxTorque);
        setMasterSlaveSpeed(motorRoles, speedCmdA, speedCmdB);

    } else { //Motor B is Master
        motorRoles.master = &FC2;
        motorRoles.slave = &FC1;
        setMasterSlaveTorque(motorRoles, maxTorque, maxTorque);
        setMasterSlaveSpeed(motorRoles, speedCmdB, speedCmdA);
    }
}

void AntiBacklashController::ProcessRunning()
{
    if (!timer.IsRunning()) {
        timer.Start();
    }
    elapsedTime = timer.TimeElapsed();
    scaledEncSpeed = encSpeedScaler(ENC1);
    scaledEncPosition = EncoderRawToDeg_F5888(ENC1);
    static int testStep = 0;
    static double targetDeg = 0.0;
    static bool moveStarted = false;

    if (!gotStartPos && int(ENC1.position) > 0) {
        startPos = EncoderRawToDeg_F5888(ENC1);
        gotStartPos = true;
    }

    if (!gotStartPos) {return;}

    if (!moveStarted) {
        switch (testStep) {
        case 0: targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 1: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 2: targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 3: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 4: antiBacklashEnabled = true; targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 5: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 6: preloadTorque = 0.25 * maxTorque; targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 7: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 8: preloadTorque = 0.35 * maxTorque; targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 9: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 10: preloadTorque = 0.45 * maxTorque; targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 11: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;

        default:
            StopAllMotors(motorRoles);
            testStep = 0;
            requestedState = "Null";
            antiBacklashEnabled = false;
            moveStarted = false;
            return;
        }
        moveStarted = true;
    }

    double currentDeg = EncoderRawToDeg_F5888(ENC1);
    double error = targetDeg - currentDeg;

    MoveToPos(motorRoles, targetDeg, antiBacklashEnabled);

    if (abs(error) < degMargin) {
        testStep++;
        moveStarted = false;
    }
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

bool AntiBacklashController::TransitionNullToRunning()
{
    if(requestedState=="Running") {
        return true;
    } else {
        return false;
    }
}

bool AntiBacklashController::TransitionRunningToNull()
{
    if(requestedState=="Null") {
        return true;
    }
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

MotorRoles AntiBacklashController::ChooseMasterSlave(double errorDeg) {
    MotorRoles roles;
    if (errorDeg > 0) {
        roles.master = &FC1;
        roles.slave = &FC2;
        return roles;
    } else {
        roles.master = &FC2;
        roles.slave = &FC1;
        return roles;
    }
}

void AntiBacklashController::MoveToPos(MotorRoles& motorRoles, double targetDeg, bool antiBacklashEnabled) {

    double currentDeg = EncoderRawToDeg_F5888(ENC1);
    double errorDeg = targetDeg - currentDeg;
    double speedRef = SpeedController(errorDeg);

    motorRoles = ChooseMasterSlave(errorDeg);
    setMasterSlaveSpeed(motorRoles, speedRef, speedRef);

    if (antiBacklashEnabled && (speedRef < maxSpeed)) {
        setMasterSlaveTorque(motorRoles, maxTorque, preloadTorque);
    } else {
        setMasterSlaveTorque(motorRoles, maxTorque, 0.0);
    }
    enableLoad(motorRoles, loadEnabled);
}

double AntiBacklashController::SpeedController(double errorDeg) {
    double absError = abs(errorDeg);
    double speed = maxSpeed;

    if (absError <= degMargin)
        return 0.0;

    if (absError < slowdownRange) {
        speed = minSpeed + (maxSpeed - minSpeed) * (absError / slowdownRange);
    }

    return speed;
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
    if (enable && debugMode) {
        roles.load->SpeedRef = speedCmdC;
        roles.load->TorqueLimitMotoring = maxTorque;
        roles.load->TorqueLimitGeneratoring = maxTorque;
        roles.load->Enable = true;
    } else if (enable) {
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

void AntiBacklashController::StopAllMotors(MotorRoles& roles) {
    setMasterSlaveTorque(roles, 0.0, 0.0);
    setMasterSlaveSpeed(roles, 0.0, 0.0);
    enableLoad(roles, false);
    enableMasterSlave(roles, false);
}

void AntiBacklashController::setLoadDrooping(MotorRoles& roles, double masterDroop, double slaveDroop, double loadDroop) {
    roles.master->LoadDrooping = masterDroop;
    roles.slave->LoadDrooping = slaveDroop;
    roles.load->LoadDrooping = loadDroop;
}
