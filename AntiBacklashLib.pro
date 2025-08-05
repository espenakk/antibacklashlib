CDPVERSION = 4.11
TYPE = library
PROJECTNAME = AntiBacklashLib

DEPS += \

HEADERS += \
    AntiBacklashController.h \
    EncoderPort.h \
    FullTest.h \
    ShaftPort.h \
    SimCmdPort.h \
    SpeedCmdSim.h \
    antibacklashlib.h \
    AntiBacklashLibBuilder.h

SOURCES += \
    AntiBacklashController.cpp \
    AntiBacklashLibBuilder.cpp \
    EncoderPort.cpp \
    FullTest.cpp \
    ShaftPort.cpp \
    SimCmdPort.cpp \
    SpeedCmdSim.cpp

DISTFILES += $$files(*.xml, true) \
    Templates/Models/AntiBacklashLib.AntiBacklashController.xml \
    Templates/Models/AntiBacklashLib.EncoderPort.xml \
    Templates/Models/AntiBacklashLib.FCIOPort.xml \
    Templates/Models/AntiBacklashLib.FullTest.xml \
    Templates/Models/AntiBacklashLib.ShaftPort.xml \
    Templates/Models/AntiBacklashLib.SimCmdPort.xml \
    Templates/Models/AntiBacklashLib.SpeedCmdSim.xml \
    Templates/Models/AntiBacklashLib.VaconFC.xml \
    Templates/Recipes/AntiBacklashLib.FC1Node.xml \
    Templates/Recipes/AntiBacklashLib.FC1Port.xml \
    Templates/Recipes/AntiBacklashLib.KublerF5888.xml

load(cdp)
