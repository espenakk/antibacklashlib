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
AntiBacklashController::AntiBacklashController(): previousSpeedRef(0.0), testPhase(0), testCycle(0), originalMaxSpeed(0.0), currentTestMaxSpeed(0.0)
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
    rampDuration.Create("rampDuration",this);
    constSpeedDuration.Create("constSpeedDuration",this);
    offset.Create("offset",this);
    slaveDroop.Create("slaveDroop",this);
    masterDroop.Create("masterDroop",this);
    enabled.Create("enabled",this);
    antiBacklashEnabled.Create("antiBacklashEnabled",this);
    timer.Create("timer",this);
    elapsedTime.Create("elapsedTime",this);
    ENC1Position.Create("ENC1Position",this);
    startAntibacklashTestButton.Create("startAntibacklashTestButton",this);
    ENC1Speed.Create("ENC1Speed",this);
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
        requestedState = "SpeedTimerTest";
    }
}

void AntiBacklashController::ProcessDebug()
{
    if (!debugMode) {
        requestedState = "Null";
    }
    elapsedTime = 42.0;
    scalePlotSignals(motorRoles);

    enableMasterSlave(motorRoles, enabled);
    enableLoad(motorRoles, false);
    double absSpeedCmd = std::abs(speedCmd);
    static double slaveTorque = adaptiveSlaveTorque(absSpeedCmd);
    // double slaveTorque = adaptiveSlaveTorque(motorRoles);
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
    scalePlotSignals(motorRoles);
    enableMasterSlave(motorRoles, true);
    enableLoad(motorRoles, true);
    static int testStep = 0;
    static double targetDeg = 0.0;
    static bool moveStarted = false;
    static double originalMaxSpeed = maxSpeed;
    static double originalDegMargin = degMargin;

    if (!moveStarted) {
        switch (testStep) {
        case 0: targetDeg = ENC1Position + 90; break;
        case 1: targetDeg = ENC1Position - 270; break;
        case 2: targetDeg = ENC1Position + 450; break;
        case 3: targetDeg = ENC1Position - 270; antiBacklashEnabled = true; break;
        case 4: targetDeg = ENC1Position + 450; break;
        case 5: targetDeg = ENC1Position - 360; antiBacklashEnabled = false; maxSpeed = maxSpeed * 2; degMargin = degMargin * 3; break;
        case 6: targetDeg = ENC1Position + 540; break;
        case 7: targetDeg = ENC1Position - 360; antiBacklashEnabled = true; break;
        case 8: targetDeg = ENC1Position + 540; break;
        case 9: targetDeg = ENC1Position - 450; antiBacklashEnabled = false; maxSpeed = maxSpeed * 2; break;
        case 10: targetDeg = ENC1Position + 630; break;
        case 11: targetDeg = ENC1Position - 450; antiBacklashEnabled = true; break;
        case 12: targetDeg = ENC1Position + 630; break;
        case 13: targetDeg = ENC1Position - 540; antiBacklashEnabled = false; maxSpeed = maxSpeed * 2; break;
        case 14: targetDeg = ENC1Position + 720; break;
        case 15: targetDeg = ENC1Position - 540; antiBacklashEnabled = true; break;
        case 16: targetDeg = ENC1Position + 720; break;

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
    // double slaveTorque = adaptiveSlaveTorque(motorRoles);
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
    elapsedTime = timer.TimeElapsed();
    scalePlotSignals(motorRoles);

    // Set antiBacklash based on testCycle
    switch (testCycle) {
    case 4:
    case 5:
    case 6:
    case 7:
        antiBacklashEnabled = true;
        break;
    default:
        antiBacklashEnabled = false;
        break;
    }

    // --- Test Sequence Logic ---
    double newSpeedCmd = 0.0;
    double timeInPhase = timer.TimeElapsed();
    double rampProgress = (rampDuration > 1e-6) ? timeInPhase / rampDuration : 1.0;
    rampProgress = std::min(rampProgress, 1.0);

    switch (testPhase) {
    // Movement 1: Negative run
    case -1: // Hold zero before and between runs
        newSpeedCmd = 0.0;
        if (timeInPhase >= constSpeedDuration) { testPhase++; timer.Start(); }
        break;
    case 0: // Ramp to negative
        newSpeedCmd = -currentTestMaxSpeed * rampProgress;
        if (timeInPhase >= rampDuration) { testPhase++; timer.Start(); }
        break;
    case 1: // Hold negative
        newSpeedCmd = -currentTestMaxSpeed;
        if (timeInPhase >= constSpeedDuration) { testPhase++; timer.Start(); }
        break;
    case 2: // Ramp to zero
        newSpeedCmd = -currentTestMaxSpeed * (1.0 - rampProgress);
        if (timeInPhase >= rampDuration) { newSpeedCmd = 0.0; testPhase++; timer.Start(); }
        break;
    // Movement 2: Positive Run
    case 3: // Ramp to positive
        newSpeedCmd = currentTestMaxSpeed * rampProgress;
        if (timeInPhase >= rampDuration) { testPhase++; timer.Start(); }
        break;
    case 4: // Hold positive
        newSpeedCmd = currentTestMaxSpeed;
        if (timeInPhase >= constSpeedDuration) { testPhase++; timer.Start(); }
        break;
    case 5: // Ramp to zero
        newSpeedCmd = currentTestMaxSpeed * (1.0 - rampProgress);
        if (timeInPhase >= rampDuration) {
            newSpeedCmd = 0.0;
            testCycle++;
            if (testCycle >= 8) {
                requestedState = "Null";
                return;
            }

            if (testCycle == 4) {
                testPhase = -1;
                currentTestMaxSpeed = originalMaxSpeed;
            } else {
                currentTestMaxSpeed *= 2.0;
                testPhase = 0;
            }
            timer.Start();
        }
        break;
    }

    speedCmd = newSpeedCmd;
    motorRoles = chooseMasterSlave(speedCmd);
    double absSpeed = std::abs(speedCmd);
    double masterSpeed = absSpeed;
    double slaveSpeed;
    if (offset != 0) {
        slaveSpeed = -absSpeed;
    } else {
        slaveSpeed = absSpeed;
    }
    motorRoles.slave->LoadDrooping = 0.0;
    motorRoles.master->LoadDrooping = 0.0;
    if (antiBacklashEnabled) {
        if (offset != 0) {
            masterSpeed = absSpeed * (1 + (offset / 100));
            slaveSpeed = -absSpeed * (1 - (offset / 100));
        }

        motorRoles.slave->LoadDrooping = slaveDroop;
        motorRoles.master->LoadDrooping = masterDroop;
    }
    double slaveTorque;
    if (offset != 0) {
        slaveTorque = maxTorque;
    } else {
        slaveTorque = adaptiveSlaveTorque(absSpeed);
    }

    setMasterSlaveSpeed(motorRoles, masterSpeed, slaveSpeed);
    setMasterSlaveTorque(motorRoles, maxTorque, slaveTorque);
    previousSpeedRef = absSpeed;
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
    if(requestedState=="PositionTest") {
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
    if(requestedState=="SpeedTimerTest") {
        timer.Start();
        testPhase = -1;
        testCycle = 0;
        originalMaxSpeed = maxSpeed;
        currentTestMaxSpeed = originalMaxSpeed;
        antiBacklashEnabled = false;
        startAntibacklashTestButton = false;
        enableMasterSlave(motorRoles, true);
        enableLoad(motorRoles, true);
        return true;
    }
    return false;
}

bool AntiBacklashController::TransitionSpeedTimerTestToNull()
{
    if(requestedState=="Null") {
        stopAllMotors(motorRoles);
        antiBacklashEnabled = false;
        maxSpeed = originalMaxSpeed;
        return true;
    }
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
    double acceleration = (speedCmd - previousSpeedRef) / ts;
    // double deltaSpeedCmd = speedCmd - previousSpeedRef;
    // double actualSpeedDiff = abs(speedCmd - encSpeedScaler(ENC1));
    if (!antiBacklashEnabled) { return 0.0; }
    if (acceleration < 0.0)  {
    // if (deltaSpeedCmd < 0.0) {
        double dynamicTorque = slaveTorqueGain * std::max(0.0, -acceleration);
        // double dynamicTorque = slaveTorqueGain * std::max(0.0, -deltaSpeedCmd) * actualSpeedDiff;
        return slaveTorqueBase + dynamicTorque;
    }
    return 0.0;
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

void AntiBacklashController::scalePlotSignals(MotorRoles& roles) {
    FC1Speed = FC1.SpeedActual;
    FC2Speed = -FC2.SpeedActual;
    FC1Torque = FC1.TorqueActual;
    FC2Torque = FC2.TorqueActual;
    FC3Torque = -std::abs(FC3.TorqueActual);

    ENC1Speed = -encSpeedScaler(ENC1);
    ENC1Position = encoderRawToDeg_F5888(ENC1);

    if (roles.master->GetNodeID() == FC1.GetNodeID()) {
        FCSpeedRef = roles.master->SpeedRef;
    } else {
        FCSpeedRef = -roles.master->SpeedRef;
    }
}
