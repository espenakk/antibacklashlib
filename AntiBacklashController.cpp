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
    velocity_deadzone.Create("velocity_deadzone",this);
    userVelocityCmdA.Create("userVelocityCmdA",this);
    userVelocityCmdB.Create("userVelocityCmdB",this);
    userVelocityCmdC.Create("userVelocityCmdC",this);
    enabled.Create("enabled",this);
    loadEnabled.Create("loadEnabled",this);
    loadTorqueLimit.Create("loadTorqueLimit",this);
    dir.Create("dir",this);
    antiBacklashEnabled.Create("antiBacklashEnabled",this);
    dirC.Create("dirC",this);

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
    FC2.TorqueLimitGeneratoring = 100;
    FC2.TorqueLimitMotoring = 100;
    FC3.PowerLimitGeneratoring = 100;
    FC3.PowerLimitMotoring = 100;
    FC3.TorqueLimitGeneratoring = loadTorqueLimit.Value();
    FC3.TorqueLimitMotoring = loadTorqueLimit.Value();

    FC1.ModeSelect = 0;
    FC2.ModeSelect = 0;

    velocity_deadzone = 5;

    auto cmdA = userVelocityCmdA.Value();
    auto cmdB = userVelocityCmdB.Value();
    auto cmdC = userVelocityCmdC.Value();

    FC1.SpeedRef = cmdA;
    FC2.SpeedRef = cmdB;
    FC3.SpeedRef = cmdC;

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

    if ((cmdA + cmdB) < velocity_deadzone.Value()) {
        FC1.TorqueLimitGeneratoring = 0;
        FC1.TorqueLimitMotoring = 0;
        FC2.TorqueLimitGeneratoring = 0;
        FC2.TorqueLimitMotoring = 0;
        // std::cout << "Standing still" << " " << FC1.SpeedRef << " " << FC2.SpeedRef << std::endl;
    } else if (dir.Value()) { //Motor A is Master
        FC1.TorqueLimitGeneratoring = 100;
        FC1.TorqueLimitMotoring = 100;

        if (antiBacklashEnabled.Value()) {
            FC2.TorqueLimitGeneratoring = preload_torque;
            FC2.TorqueLimitMotoring = preload_torque;
        } else {
            FC2.TorqueLimitGeneratoring = 0;
            FC2.TorqueLimitMotoring = 0;
        }

        if (cmdC < velocity_deadzone.Value()) {
            FC3.SpeedRef = cmdA/10;
            dirC = false;
        }

        // std::cout << "Motor A is running" << " " << FC1.SpeedRef << " " << FC2.SpeedRef << std::endl;
    } else { //Motor B is Master
        FC2.TorqueLimitGeneratoring = 100;
        FC2.TorqueLimitMotoring = 100;

        if (antiBacklashEnabled.Value()) {
            FC1.TorqueLimitGeneratoring = preload_torque;
            FC1.TorqueLimitMotoring = preload_torque;
        } else {
            FC1.TorqueLimitGeneratoring = 0;
            FC1.TorqueLimitMotoring = 0;
        }

        if (cmdC < velocity_deadzone.Value()) {
            FC3.SpeedRef = cmdB/10;
            dirC = true;
        }

        // std::cout << "Motor B is running" << " " << FC1.SpeedRef << " " << FC2.SpeedRef << std::endl;
    }

    error = positionSP - ENC1.position;
    proportional = kp_ * error;
    integral = ki_ * accumulatedError;
    derivative = kd_ * (error - previousError);
    output = proportional + integral + derivative;
    // if (output <)
    previousError = error;
    accumulatedError += error;
}
