#include "EncoderPort.h"

using namespace AntiBacklashLib;
/*!
  \class AntiBacklashLib::EncoderPort
  \inmodule AntiBacklashLib

  \section1 Usage

  Add documentation for EncoderPort here.
*/

/*!
  \internal
  \brief Creates the instance. Called after constructor.
*/
void EncoderPort::Create(const char* shortName, CDPComponent* parent)
{
    CDPPort::Create(shortName, parent);
    position.Create("position", this);
}
