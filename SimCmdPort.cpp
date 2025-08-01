#include "SimCmdPort.h"

using namespace AntiBacklashLib;
/*!
  \class AntiBacklashLib::ABCTestSimPort
  \inmodule AntiBacklashLib

  \section1 Usage

  Add documentation for ABCTestSimPort here.
*/

/*!
  \internal
  \brief Creates the instance. Called after constructor.
*/
void SimCmdPort::Create(const char* shortName, CDPComponent* parent)
{
    CDPPort::Create(shortName, parent);
    SpeedCMD.Create("SpeedCMD", this);
    AntiBacklashEnabled.Create("AntiBacklashEnabled", this);
    EnableFCs.Create("EnableFCs", this);
}
