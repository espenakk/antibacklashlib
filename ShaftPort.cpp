#include "ShaftPort.h"

using namespace AntiBacklashLib;
/*!
  \class AntiBacklashLib::ShaftPort
  \inmodule AntiBacklashLib

  \section1 Usage

  Add documentation for ShaftPort here.
*/

/*!
  \internal
  \brief Creates the instance. Called after constructor.
*/
void ShaftPort::Create(const char* shortName, CDPComponent* parent)
{
    CDPPort::Create(shortName, parent);
    ShaftAngle.Create("ShaftAngle", this);
    ShaftRounds.Create("ShaftRounds", this);
}
