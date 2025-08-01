#include "SpeedCmdSim.h"

using namespace AntiBacklashLib;
/*!
  \class AntiBacklashLib::SpeedCmdSim
  \inmodule AntiBacklashLib

  \section1 Usage

  Add documentation for SpeedCmdSim here.
*/

/*!
  \internal
  \brief Component constructor. The first function to be called. Can be used to initialize member variables, etc.
*/
SpeedCmdSim::SpeedCmdSim()
{
}

/*!
  \internal
  \brief Component destructor. The last function to be called. Typically used to clean up when stopping, like freeing
  allocated memory, etc.
*/
SpeedCmdSim::~SpeedCmdSim()
{
}

/*!
  \internal
  \brief Creates the component instance and other related CDP objects. Called after constructor

  Note, that this function is normally filled automatically by the code generator.
*/
void SpeedCmdSim::Create(const char* fullName)
{
    CDPComponent::Create(fullName);
    SimCmd.Create("SimCmd",this);
    RampDuration.Create("RampDuration",this);
    ConstSpeedDuration.Create("ConstSpeedDuration",this);
    MaxSpeed.Create("MaxSpeed",this);
    Start.Create("Start",this);
}

/*!
  \internal
  \brief Creates a model instance for this class and fills model data. Registers messages, states and state transitions.

  Note, that this function is normally filled automatically by the code generator.
*/
void SpeedCmdSim::CreateModel()
{
    CDPComponent::CreateModel();

    RegisterStateProcess("Null", (CDPCOMPONENT_STATEPROCESS) &SpeedCmdSim::ProcessNull, "Initial Null state");
    RegisterStateProcess("Running",(CDPCOMPONENT_STATEPROCESS)&SpeedCmdSim::ProcessRunning,"");
    RegisterStateTransitionHandler("Null","Running",(CDPCOMPONENT_STATETRANSITIONHANDLER)&SpeedCmdSim::TransitionNullToRunning,"");
    RegisterStateTransitionHandler("Running","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&SpeedCmdSim::TransitionRunningToNull,"");
}

/*!
  \internal
  \brief Configures the component by reading the configuration. Called after Create()/CreateModel().

  Note, that this function is normally filled automatically by the code generator.
*/
void SpeedCmdSim::Configure(const char* componentXML)
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
void SpeedCmdSim::ProcessNull()
{
    if (Start) {
        requestedState = "Running";
    }
}



void SpeedCmdSim::ProcessRunning()
{
    // Set antiBacklash based on testCycle
    switch (testCycle) {
    case 4:
    case 5:
    case 6:
    case 7:
        SimCmd.AntiBacklashEnabled = true;
        break;
    default:
        SimCmd.AntiBacklashEnabled = false;
        break;
    }

    // Test Sequence
    double newSpeedCMD = 0.0;
    double timeInPhase = timer.TimeElapsed();
    double rampProgress = (RampDuration > 1e-6) ? timeInPhase / RampDuration : 1.0;
    rampProgress = std::min(rampProgress, 1.0);

    switch (testPhase) {
    // Movement 1: Negative run
    case -1: // Hold zero before and between runs
        newSpeedCMD = 0.0;
        if (timeInPhase >= ConstSpeedDuration) { testPhase++; timer.Start(); }
        break;
    case 0: // Ramp to negative
        newSpeedCMD = -currentTestMaxSpeed * rampProgress;
        if (timeInPhase >= RampDuration) { testPhase++; timer.Start(); }
        break;
    case 1: // Hold negative
        newSpeedCMD = -currentTestMaxSpeed;
        if (timeInPhase >= ConstSpeedDuration) { testPhase++; timer.Start(); }
        break;
    case 2: // Ramp to zero
        newSpeedCMD = -currentTestMaxSpeed * (1.0 - rampProgress);
        if (timeInPhase >= RampDuration) { newSpeedCMD = 0.0; testPhase++; timer.Start(); }
        break;
    // Movement 2: Positive Run
    case 3: // Ramp to positive
        newSpeedCMD = currentTestMaxSpeed * rampProgress;
        if (timeInPhase >= RampDuration) { testPhase++; timer.Start(); }
        break;
    case 4: // Hold positive
        newSpeedCMD = currentTestMaxSpeed;
        if (timeInPhase >= ConstSpeedDuration) { testPhase++; timer.Start(); }
        break;
    case 5: // Ramp to zero
        newSpeedCMD = currentTestMaxSpeed * (1.0 - rampProgress);
        if (timeInPhase >= RampDuration) {
            newSpeedCMD = 0.0;
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
    SimCmd.SpeedCMD = newSpeedCMD;
}



bool SpeedCmdSim::TransitionNullToRunning()
{
    if(requestedState=="Running") {
        timer.Start();
        testPhase = -1;
        testCycle = 0;
        originalMaxSpeed = MaxSpeed;
        currentTestMaxSpeed = originalMaxSpeed;
        SimCmd.AntiBacklashEnabled = false;
        Start = false;
        SimCmd.EnableFCs = true;
        return true;
    }
        return false;
}



bool SpeedCmdSim::TransitionRunningToNull()
{
    if(requestedState=="Null") {
        SimCmd.EnableFCs = false;
        SimCmd.AntiBacklashEnabled = false;
        MaxSpeed = originalMaxSpeed;
        return true;
    }
    return false;
}
