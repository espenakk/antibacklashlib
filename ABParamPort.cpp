#include "ABParamPort.h"

using namespace AntiBacklashLib;
/*!
  \class AntiBacklashLib::ABParamPort
  \inmodule AntiBacklashLib

  \section1 Usage

  Add documentation for ABParamPort here.
*/

/*!
  \internal
  \brief Creates the instance. Called after constructor.
*/
void ABParamPort::Create(const char* shortName, CDPComponent* parent)
{
    CDPPort::Create(shortName, parent);
    AntiBacklashMode.Create("AntiBacklashMode", this);
    LoadTorqueLimit.Create("LoadTorqueLimit", this);
    MaxTorque.Create("MaxTorque", this);
    SlaveTorqueBase.Create("SlaveTorqueBase", this);
    SlaveTorqueGain.Create("SlaveTorqueGain", this);
    Offset.Create("Offset", this);
    SlaveDroop.Create("SlaveDroop", this);
    MasterDroop.Create("MasterDroop", this);
}
