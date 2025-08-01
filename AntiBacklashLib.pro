CDPVERSION = 4.11
TYPE = library
PROJECTNAME = AntiBacklashLib

DEPS += \

HEADERS += \
    AntiBacklashController.h \
    EncoderPort.h \
    SimCmdPort.h \
    SpeedCmdSim.h \
    antibacklashlib.h \
    AntiBacklashLibBuilder.h

SOURCES += \
    AntiBacklashController.cpp \
    AntiBacklashLibBuilder.cpp \
    EncoderPort.cpp \
    SimCmdPort.cpp \
    SpeedCmdSim.cpp

DISTFILES += $$files(*.xml, true) \
    Templates/Models/AntiBacklashLib.AntiBacklashController.xml \
    Templates/Models/AntiBacklashLib.EncoderPort.xml \
    Templates/Models/AntiBacklashLib.SimCmdPort.xml \
    Templates/Models/AntiBacklashLib.SpeedCmdSim.xml \
    Templates/Recipes/AntiBacklashLib.FC1Node.xml \
    Templates/Recipes/AntiBacklashLib.FC1Port.xml \
    Templates/Recipes/AntiBacklashLib.KublerF5888.xml \
    Templates/Recipes/AntiBacklashLib.VaconFC1.xml

load(cdp)
