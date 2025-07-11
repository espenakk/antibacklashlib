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
    FC1.Create("FC1",this);
    FC2.Create("FC2",this);
    FC3.Create("FC3",this);
    ENC1.Create("ENC1",this);
    preload_torque.Create("preload_torque",this);
    userVelocityCmdA.Create("userVelocityCmdA",this);
    userVelocityCmdB.Create("userVelocityCmdB",this);
    userVelocityCmdC.Create("userVelocityCmdC",this);
    enabled.Create("enabled",this);
    loadEnabled.Create("loadEnabled",this);
    loadTorqueLimit.Create("loadTorqueLimit",this);
    dir.Create("dir",this);
    antiBacklashEnabled.Create("antiBacklashEnabled",this);
    dirC.Create("dirC",this);
    timer.Create("timer",this);
    elapsedTime.Create("elapsedTime",this);
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
    FC1.PowerLimitGeneratoring = 100;
    FC1.PowerLimitMotoring = 100;
    FC2.PowerLimitGeneratoring = 100;
    FC2.PowerLimitMotoring = 100;
    FC3.PowerLimitGeneratoring = 100;
    FC3.PowerLimitMotoring = 100;

    FC1.ModeSelect = 3;
    FC2.ModeSelect = 3;
    FC3.ModeSelect = 3;

    scaledEncSpeed = encSpeedScaler();

    if (startAntibacklashTestButton || runningAntiBacklashTestScript) {

        runningAntiBacklashTestScript = true;
        FC1.Enable = FC2.Enable = FC3.Enable = true;

        if (!timer.IsRunning()) {
            timer.Start();
        }
        elapsedTime = timer.TimeElapsed();

        // tests to run:
        //  run forward without load -> change dir
        //  run backwards without load -> change dir
        //  run forward with load -> change dir
        //  run backwards with load -> change dir
        //  activate backlash and redo
        antibacklashTestScript(elapsedTime, 20, 30);

    } else {
        elapsedTime = 0.0;
        debugMode();
    }
}



void AntiBacklashController::antibacklashTestScript(double t, int speedNoLoad = 15, int speedLoad = 20) {
    int time_interval = (int)t / 5;

    switch (time_interval) {
        case 0: // 0 to 4.99 seconds
            exportData = false;
            FC1.SpeedRef = speedNoLoad;
            FC2.SpeedRef = speedNoLoad;

            FC1.TorqueLimitGeneratoring = 10;
            FC1.TorqueLimitMotoring = 10;
            FC2.TorqueLimitGeneratoring = 0;
            FC2.TorqueLimitMotoring = 0;

            FC3.TorqueLimitGeneratoring = 10;
            FC3.TorqueLimitMotoring = 10;
            break;

        case 1: // 5 to 9.99 seconds
            FC1.SpeedRef = speedNoLoad;
            FC2.SpeedRef = speedNoLoad;

            FC1.TorqueLimitGeneratoring = 0;
            FC1.TorqueLimitMotoring = 0;
            FC2.TorqueLimitGeneratoring = 10;
            FC2.TorqueLimitMotoring = 10;

            FC3.TorqueLimitGeneratoring = 10;
            FC3.TorqueLimitMotoring = 10;
            break;

        case 2: // 10 to 14.99 seconds
            exportData = true;
            FC1.SpeedRef = speedLoad;
            FC2.SpeedRef = speedLoad;
            FC3.SpeedRef = speedLoad / 10;
            dirC = false;

            FC1.TorqueLimitGeneratoring = 10;
            FC1.TorqueLimitMotoring = 10;
            FC2.TorqueLimitGeneratoring = 0;
            FC2.TorqueLimitMotoring = 0;

            FC3.TorqueLimitGeneratoring = 10;
            FC3.TorqueLimitMotoring = 10;
            break;

        case 3: // 15 to 19.99 seconds
            FC1.SpeedRef = speedLoad;
            FC2.SpeedRef = speedLoad;
            FC3.SpeedRef = speedLoad / 10;
            dirC = true;

            FC1.TorqueLimitGeneratoring = 0;
            FC1.TorqueLimitMotoring = 0;
            FC2.TorqueLimitGeneratoring = 10;
            FC2.TorqueLimitMotoring = 10;

            FC3.TorqueLimitGeneratoring = 10;
            FC3.TorqueLimitMotoring = 10;
            break;

        case 4: // 20 to 24.99 seconds (ANTI BACKLASH!!!)
            exportData = false;
            antiBacklashEnabled = true;
            FC1.SpeedRef = speedNoLoad;
            FC2.SpeedRef = speedNoLoad;

            FC1.TorqueLimitGeneratoring = 10;
            FC1.TorqueLimitMotoring = 10;
            FC2.TorqueLimitGeneratoring = 0;
            FC2.TorqueLimitMotoring = 0;

            FC3.TorqueLimitGeneratoring = 10;
            FC3.TorqueLimitMotoring = 10;
            break;

        case 5: // 25 to 29.99 seconds
            FC1.SpeedRef = speedNoLoad;
            FC2.SpeedRef = speedNoLoad;

            FC1.TorqueLimitGeneratoring = 0;
            FC1.TorqueLimitMotoring = 0;
            FC2.TorqueLimitGeneratoring = 10;
            FC2.TorqueLimitMotoring = 10;

            FC3.TorqueLimitGeneratoring = 10;
            FC3.TorqueLimitMotoring = 10;
            break;

        case 6: // 30 to 34.99 seconds
            exportData = true;
            FC1.SpeedRef = speedLoad;
            FC2.SpeedRef = speedLoad;
            FC3.SpeedRef = speedLoad / 10;
            dirC = false;

            FC1.TorqueLimitGeneratoring = 10;
            FC1.TorqueLimitMotoring = 10;
            FC2.TorqueLimitGeneratoring = 0;
            FC2.TorqueLimitMotoring = 0;

            FC3.TorqueLimitGeneratoring = 10;
            FC3.TorqueLimitMotoring = 10;
            break;

        case 7: // 35 to 39.99 seconds
            FC1.SpeedRef = speedLoad;
            FC2.SpeedRef = speedLoad;
            FC3.SpeedRef = speedLoad / 10;
            dirC = true;

            FC1.TorqueLimitGeneratoring = 0;
            FC1.TorqueLimitMotoring = 0;
            FC2.TorqueLimitGeneratoring = 10;
            FC2.TorqueLimitMotoring = 10;

            FC3.TorqueLimitGeneratoring = 10;
            FC3.TorqueLimitMotoring = 10;
            break;

        default: // 45 seconds or more
            exportData = false;
            timer.Reset();
            FC1.Enable = FC2.Enable = FC3.Enable = false;
            antiBacklashEnabled = false;
            runningAntiBacklashTestScript = false;
            break;
    }
}



void AntiBacklashController::debugMode() {
    FC3.TorqueLimitGeneratoring = loadTorqueLimit.Value();
    FC3.TorqueLimitMotoring = loadTorqueLimit.Value();

    FC1.SpeedRef = userVelocityCmdA.Value();
    FC2.SpeedRef = userVelocityCmdB.Value();
    FC3.SpeedRef = userVelocityCmdC.Value();
    FC3.TorqueLimitGeneratoring = 10;
    FC3.TorqueLimitMotoring = 10;

    if (enabled.Value()) {
        FC1.Enable = true;
        FC2.Enable = true;
    } else {
        FC1.Enable = false;
        FC2.Enable = false;
    }

    if (loadEnabled.Value()) {
        FC3.Enable = true;
    } else {
        FC3.Enable = false;
    }

    if (dir.Value()) { //Motor A is Master
        FC1.TorqueLimitGeneratoring = 10;
        FC1.TorqueLimitMotoring = 10;
        FC2.TorqueLimitGeneratoring = preload_torque;
        FC2.TorqueLimitMotoring = preload_torque;

    } else { //Motor B is Master
        FC2.TorqueLimitGeneratoring = 10;
        FC2.TorqueLimitMotoring = 10;
        FC1.TorqueLimitGeneratoring = preload_torque;
        FC1.TorqueLimitMotoring = preload_torque;
    }
}

int AntiBacklashController::encSpeedScaler() {
    return ENC1.speed / 14;
}
