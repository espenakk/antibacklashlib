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
    ABParams.Create("ABParams",this);
    NumTests.Create("NumTests",this);
    GainMin.Create("GainMin",this);
    GainMax.Create("GainMax",this);
    BaseMin.Create("BaseMin",this);
    BaseMax.Create("BaseMax",this);
    OffsetMin.Create("OffsetMin",this);
    OffsetMax.Create("OffsetMax",this);
    DroopMin.Create("DroopMin",this);
    DroopMax.Create("DroopMax",this);
    AntiBacklashMode.Create("AntiBacklashMode",this);
    LoadTorqueLimit.Create("LoadTorqueLimit",this);
    MaxTorque.Create("MaxTorque",this);
    SlaveTorqueBase.Create("SlaveTorqueBase",this);
    SlaveTorqueGain.Create("SlaveTorqueGain",this);
    Offset.Create("Offset",this);
    SlaveDroop.Create("SlaveDroop",this);
    MasterDroop.Create("MasterDroop",this);
    m_Start.Create("m_Start",this);
    m_MaxSpeed.Create("m_MaxSpeed",this);
    Start.Create("Start",this);
    MaxSpeed.Create("MaxSpeed",this);
    Enable.Create("Enable",this);
    IsRunning.Create("IsRunning",this);
    CurrentStatus.Create("CurrentStatus",this);
    TotalTests.Create("TotalTests",this);
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

    ABParams.AntiBacklashMode = AntiBacklashMode;
    ABParams.LoadTorqueLimit = LoadTorqueLimit;
    ABParams.MaxTorque = MaxTorque;
    ABParams.SlaveTorqueBase = SlaveTorqueBase;
    ABParams.SlaveTorqueGain = SlaveTorqueGain;
    ABParams.Offset = Offset;
    ABParams.SlaveDroop = SlaveDroop;
    ABParams.MasterDroop = MasterDroop;
    Start = m_Start;
    MaxSpeed = m_MaxSpeed;
}

void FullTest::ProcessRunning()
{
    if (!Enable) {
        requestedState = "Null";
        return;
    }
    TotalTests = NumTests * 4;
    CurrentStatus = currentParamStep + (currentMode * int(NumTests));

    // The main sequencer logic is a state machine within the "Running" state.
    switch (sequence) {
    case 0: {
        // if (currentMode >= 4) {
        //     CDPMessage("Full test sequence completed successfully.\n");
        //     requestedState = "Null"; // All done, go back to Null
        //     return;
        // }
        if (currentMode >= 6) {
            CDPMessage("Full test sequence completed successfully.\n");
            requestedState = "Null"; // All done, go back to Null
            return;
        }

        if (NumTests < 1) {
            CDPMessage("Skipping Mode %d (NumTestsPerMode is %d).\n", currentMode, int(NumTests));
            currentMode++;
            currentParamStep = 0;
            sequence = 0;
            return;
        }

        CDPMessage("Setting up test for Mode: %d, Param Step: %d of %d\n", currentMode, currentParamStep + 1, int(NumTests));

        ABParams.AntiBacklashMode = currentMode;
        double range;
        double value;

        switch (currentMode) {
        // case 0: // AdaptiveTorque Mode Test
        //     range = GainMax - GainMin;
        //     value = (NumTests == 1) ? (GainMin + range / 2.0) : (GainMin + currentParamStep * (range / (NumTests - 1)));
        //     ABParams.Offset = 0;
        //     ABParams.SlaveTorqueGain = value;
        //     ABParams.SlaveTorqueBase = value / 2;
        //     break;
        // case 1: // ConstTorque Mode Test
        //     range = BaseMax - BaseMin;
        //     value = (NumTests == 1) ? (BaseMin + range / 2.0) : (BaseMin + currentParamStep * (range / (NumTests - 1)));
        //     ABParams.Offset = 0;
        //     ABParams.SlaveTorqueGain = 0;
        //     ABParams.SlaveTorqueBase = value;
        //     break;
        // case 2: // SpeedCmdOffset Mode Test
        //     range = OffsetMax - OffsetMin;
        //     value = (NumTests == 1) ? (OffsetMin + range / 2.0) : (OffsetMin + currentParamStep * (range / (NumTests - 1)));
        //     ABParams.Offset = value;
        //     ABParams.SlaveTorqueGain = 0;
        //     ABParams.SlaveTorqueBase = MaxTorque;
        //     break;
        // case 3: // SlaveDrooping Mode Test
        //     range = DroopMax - DroopMin;
        //     value = (NumTests == 1) ? (DroopMin + range / 2.0) : (DroopMin + currentParamStep * (range / (NumTests - 1)));
        //     ABParams.Offset = 0;
        //     ABParams.SlaveTorqueGain = 0;
        //     ABParams.SlaveTorqueBase = MaxTorque;
        //     SlaveDroop = value;
        //     break;
        case 4: // ActualPositionOffset Mode Test
            // No params to set
            break;
        case 5: // SlaveSpeedRefDelay Mode Test
            // No params to set
            break;
        // --- Not yet implemented ---
        // case 6: // ConstrainedSlaceAcceleration Mode Test
        //     break;
        default:
            Enable = false;
            break;
        }

        Start = true;
        sequence = 1;
        break;
    }

    case 1: // To ensure it actually started.
        if (IsRunning) {
            Start = false;
            sequence = 2;
        }
        break;

    case 2:
        if (!IsRunning) {
            CDPMessage("Motion profile finished. Advancing to next test.\n");

            currentParamStep++;

            if (currentParamStep >= NumTests) {
                currentParamStep = 0;
                currentMode++;
            }
            sequence = 0;
        }
        break;
    }
}
bool FullTest::TransitionNullToRunning()
{
    if (requestedState == "Running") {
        currentMode = 4;
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
        ABParams.AntiBacklashMode = -1;
        ABParams.MasterDroop = 0;
        ABParams.SlaveDroop = 0;
        ABParams.SlaveTorqueBase = 3;
        ABParams.SlaveTorqueGain = 5;
        ABParams.Offset = 1;
        Enable = false;
        CDPMessage("FullTest returning to Null state.\n");
        return true;
    }
    return false;
}
