#include "AntiBacklashController.h"
#include <iostream>
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
    exportData = false;
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
    exportData.Create("exportData",this);
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
    scaledEncSpeed = encSpeedScaler();
    scaledEncPosition = EncoderRawToDeg_F5888(ENC1);

    MasterSlave roles;

    FC1.SpeedRef = speedCmdA;
    FC2.SpeedRef = speedCmdB;
    FC3.SpeedRef = speedCmdC;

    enableMasterSlave(FC1, FC2, enabled);

    enableLoad(FC3, loadEnabled);

    if (dir) { //Motor A is Master
        roles.master = &FC1;
        roles.slave = &FC2;
        setMasterSlaveTorque(roles, kMaxTorque, kMaxTorque);

    } else { //Motor B is Master
        roles.master = &FC2;
        roles.slave = &FC1;
        setMasterSlaveTorque(roles, kMaxTorque, kMaxTorque);
    }
}





void AntiBacklashController::ProcessRunning()
{
    if (!timer.IsRunning()) {
        timer.Start();
    }
    elapsedTime = timer.TimeElapsed();
    scaledEncSpeed = encSpeedScaler();
    scaledEncPosition = EncoderRawToDeg_F5888(ENC1);
    static int testStep = 0;
    static double targetDeg = 0.0;
    static bool moveStarted = false;

    if (!gotStartPos && ENC1.position > 0) {
        startPos = EncoderRawToDeg_F5888(ENC1);
        gotStartPos = true;
    }

    if (!gotStartPos) {return;}

    if (!moveStarted) {
        switch (testStep) {
        // case 0: targetDeg = EncoderRawToDeg_F5888(ENC1) + 360; loadEnabled = false; break;
        case 0: targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 1: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 2: targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 3: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 4: antiBacklashEnabled = true; targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        // case 4: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = false; break;
        // case 5: targetDeg = EncoderRawToDeg_F5888(ENC1) - 450; loadEnabled = false; break;
        case 5: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 6: preloadTorque = 0.25 * kMaxTorque; targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 7: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 8: preloadTorque = 0.35 * kMaxTorque; targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 9: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;
        case 10: preloadTorque = 0.45 * kMaxTorque; targetDeg = EncoderRawToDeg_F5888(ENC1) - 270; loadEnabled = true; break;
        case 11: targetDeg = EncoderRawToDeg_F5888(ENC1) + 450; loadEnabled = true; break;

        default:
            StopAllMotors();
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

    MoveToPos(targetDeg, antiBacklashEnabled);

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

void AntiBacklashController::StopAllMotors() {
    FC1.SpeedRef = FC2.SpeedRef = 0;
    FC1.TorqueLimitMotoring = FC2.TorqueLimitMotoring = 0;
    FC1.TorqueLimitGeneratoring = FC2.TorqueLimitGeneratoring = 0;
}

MasterSlave AntiBacklashController::ChooseMasterSlave(double errorDeg, double speed) {
    MasterSlave roles;
    if (errorDeg > 0) {
        FC1.SpeedRef  = FC2.SpeedRef = speed;
        FC1.TorqueLimitMotoring = FC1.TorqueLimitGeneratoring = kMaxTorque;

        // FC2.SpeedRef = -speed;
        FC2.TorqueLimitMotoring = FC2.TorqueLimitGeneratoring = 0;
        roles.master = &FC1;
        roles.slave = &FC2;
        return roles;
    } else {
        // FC1.SpeedRef = -speed;
        FC1.TorqueLimitMotoring = FC1.TorqueLimitGeneratoring = 0;

        FC2.SpeedRef = FC1.SpeedRef = speed;
        FC2.TorqueLimitMotoring = FC2.TorqueLimitGeneratoring = kMaxTorque;
        roles.master = &FC2;
        roles.slave = &FC1;
        return roles;
    }
}

void AntiBacklashController::setSpeedSP() {

}

// move to a given position in degrees
void AntiBacklashController::MoveToPos(double targetDeg, bool antiBacklashEnabled) {
    // defines FC1 pos dir and FC2 neg dir

    double currentDeg = EncoderRawToDeg_F5888(ENC1);
    double errorDeg = targetDeg - currentDeg;
    double speed = SpeedController(errorDeg);

    MasterSlave motorRoles = ChooseMasterSlave(errorDeg, speed);

    // bool allowPreload = antiBacklashEnabled && (abs(speed) < velocityDeadzone || velocityDeadzone == 0);
    bool allowPreload = antiBacklashEnabled && (abs(speed) < (2.0 * M_PI / 5));
    // bool allowPreload = antiBacklashEnabled;

    setMasterSlaveTorque(motorRoles, kMaxTorque, preloadTorque);
    enableLoad(FC3, loadEnabled);
}

double AntiBacklashController::SpeedController(double errorDeg) {
    const double maxSpeed = 2.0 * M_PI / 5;
    const double minSpeed = 2.0 * M_PI / 20;
    const double slowdownRange = 50.0;

    double absError = abs(errorDeg);

    if (absError <= degMargin)
        return 0.0;

    double speed = maxSpeed;

    if (absError < slowdownRange) {
        speed = minSpeed + (maxSpeed - minSpeed) * (absError / slowdownRange);
    }

    return speed;
}

void AntiBacklashController::setMasterSlaveTorque(MasterSlave roles, double masterTorque, double slaveTorque) {
    roles.master->TorqueLimitGeneratoring = roles.master->TorqueLimitMotoring = masterTorque;
    roles.slave->TorqueLimitGeneratoring = roles.slave->TorqueLimitMotoring = slaveTorque;
}

void AntiBacklashController::enableMasterSlave(VaconLib::VaconMarineAppFCPort& master, VaconLib::VaconMarineAppFCPort& slave, bool enable) {
    if (enable) {
        master.Enable = slave.Enable = true;
    } else {
        master.Enable = slave.Enable = false;
    }
}

void AntiBacklashController::enableLoad(VaconLib::VaconMarineAppFCPort& load, bool enable) {
    if (enable) {
        load.SpeedRef = 0.0;
        load.TorqueLimitMotoring = load.TorqueLimitGeneratoring = kLoadTorqueLimit;
        load.Enable = true;
    } else {
        load.Enable = false;
        load.TorqueLimitGeneratoring = load.TorqueLimitMotoring = kMaxTorque;
    }
}
