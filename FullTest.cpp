#include "FullTest.h"

using namespace AntiBacklashLib;
/*!
  \class AntiBacklashLib::FullTest
  \inmodule AntiBacklashLib

  \section1 Usage

  Add documentation for FullTest here.
*/

/*!
  \internal
  \brief Component constructor. The first function to be called. Can be used to initialize member variables, etc.
*/
FullTest::FullTest()
{
}

/*!
  \internal
  \brief Component destructor. The last function to be called. Typically used to clean up when stopping, like freeing
  allocated memory, etc.
*/
FullTest::~FullTest()
{
}

/*!
  \internal
  \brief Creates the component instance and other related CDP objects. Called after constructor

  Note, that this function is normally filled automatically by the code generator.
*/
void FullTest::Create(const char* fullName)
{
    CDPComponent::Create(fullName);
    NumTests.Create("NumTests",this);
    AntiBacklashMode.Create("AntiBacklashMode",this);
    LoadTorqueLimit.Create("LoadTorqueLimit",this);
    MaxTorque.Create("MaxTorque",this);
    SlaveTorqueBase.Create("SlaveTorqueBase",this);
    SlaveTorqueGain.Create("SlaveTorqueGain",this);
    Offset.Create("Offset",this);
    SlaveDroop.Create("SlaveDroop",this);
    MasterDroop.Create("MasterDroop",this);
    Start.Create("Start",this);
    MaxSpeed.Create("MaxSpeed",this);
    Enable.Create("Enable",this);
    IsRunning.Create("IsRunning",this);
}

/*!
  \internal
  \brief Creates a model instance for this class and fills model data. Registers messages, states and state transitions.

  Note, that this function is normally filled automatically by the code generator.
*/
void FullTest::CreateModel()
{
    CDPComponent::CreateModel();

    RegisterStateProcess("Null", (CDPCOMPONENT_STATEPROCESS) &FullTest::ProcessNull, "Initial Null state");
    RegisterStateProcess("Running",(CDPCOMPONENT_STATEPROCESS)&FullTest::ProcessRunning,"");
    RegisterStateTransitionHandler("Null","Running",(CDPCOMPONENT_STATETRANSITIONHANDLER)&FullTest::TransitionNullToRunning,"");
    RegisterStateTransitionHandler("Running","Null",(CDPCOMPONENT_STATETRANSITIONHANDLER)&FullTest::TransitionRunningToNull,"");
}

/*!
  \internal
  \brief Configures the component by reading the configuration. Called after Create()/CreateModel().

  Note, that this function is normally filled automatically by the code generator.
*/
void FullTest::Configure(const char* componentXML)
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
void FullTest::ProcessNull()
{
    if (Enable) {
        requestedState = "Running";
    }
}

void FullTest::ProcessRunning()
{
    if (!Enable) {
        requestedState = "Null";
        return;
    }

    // --- Test Configuration ---
    // Define the parameter ranges for each mode.
    const double GAIN_MIN = 1,  GAIN_MAX = 5;
    const double BASE_MIN = 2.5,  BASE_MAX = 4.5;
    const double OFFSET_MIN = 0.5, OFFSET_MAX = 2.5;
    const double DROOP_MIN = 1.0,  DROOP_MAX = 10.0;

    // The main sequencer logic is a state machine within the "Running" state.
    switch (sequence)
    {
    case 0: // State 0: Setup the test
    {
        // Check if all modes are complete
        if (currentMode >= 4) {
            CDPMessage("Full test sequence completed successfully.\n");
            requestedState = "Null"; // All done, go back to Null
            return;
        }

        // If NumTestsPerMode is 0 or less, skip this mode.
        if (NumTests < 1) {
            CDPMessage("Skipping Mode %d (NumTestsPerMode is %d).\n", currentMode, int(NumTests));
            currentMode++;
            currentParamStep = 0; // Reset for next mode
            sequence = 0;         // Stay in setup state for the next mode
            return;
        }

        CDPMessage("Setting up test for Mode: %d, Param Step: %d of %d\n", currentMode, currentParamStep + 1, int(NumTests));

        AntiBacklashMode = currentMode; // Modes 0, 1, 2, 3

        // Calculate the parameter value for the current step symmetrically within its range.
        switch (currentMode) {
        case 0: // DecelTorque Mode Test -> Vary SlaveTorqueGain
        {
            double range = GAIN_MAX - GAIN_MIN;
            // If only one test, use the midpoint. Otherwise, distribute evenly.
            double value = (NumTests == 1) ? (GAIN_MIN + range / 2.0) : (GAIN_MIN + currentParamStep * (range / (NumTests - 1)));
            SlaveTorqueGain = value;
            break;
        }
        case 1: // ConstTorque Mode Test -> Vary SlaveTorqueBase
        {
            double range = BASE_MAX - BASE_MIN;
            double value = (NumTests == 1) ? (BASE_MIN + range / 2.0) : (BASE_MIN + currentParamStep * (range / (NumTests - 1)));
            SlaveTorqueBase = value;
            break;
        }
        case 2: // SpeedCmdOffset Mode Test -> Vary Offset
        {
            double range = OFFSET_MAX - OFFSET_MIN;
            double value = (NumTests == 1) ? (OFFSET_MIN + range / 2.0) : (OFFSET_MIN + currentParamStep * (range / (NumTests - 1)));
            Offset = value;
            break;
        }
        case 3: // SlaveDrooping Mode Test -> Vary SlaveDroop
        {
            double range = DROOP_MAX - DROOP_MIN;
            double value = (NumTests == 1) ? (DROOP_MIN + range / 2.0) : (DROOP_MIN + currentParamStep * (range / (NumTests - 1)));
            SlaveDroop = value;
            break;
        }
        default:
        {
            Enable = false;
            break;
        }
        }

        // Trigger the SpeedCmdSim to start its motion profile
        Start = true;

        // Transition to the next state to wait for completion
        sequence = 1;
        break;
    }

    case 1: // State 1: Wait for the test to finish
    {
        // First, de-assert the start trigger.
        Start = false;

        // Wait until the SpeedCmdSim component reports it is no longer running.
        if (!IsRunning) {
            CDPMessage("Motion profile finished. Advancing to next test.\n");

            // The motion is done, so advance the parameter step.
            currentParamStep++;

            // If all parameter steps for this mode are done, move to the next mode.
            if (currentParamStep >= NumTests) {
                currentParamStep = 0;
                currentMode++;
            }

            // Go back to the setup state for the next test run.
            sequence = 0;
        }
        break;
    }
    }
}

bool FullTest::TransitionNullToRunning()
{
    if (requestedState == "Running") {
        currentMode = 0;
        currentParamStep = 0;
        sequence = 0;
        CDPMessage("FullTest entering Running state.\n");
        return true;
    }
    return false;
}

bool FullTest::TransitionRunningToNull()
{
    if (requestedState == "Null") {
        Start = false;
        AntiBacklashMode = -1;
        CDPMessage("FullTest returning to Null state.\n");
        return true;
    }
    return false;
}
