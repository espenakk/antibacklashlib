CDPVERSION = 4.11
TYPE = library
PROJECTNAME = AntiBacklashLib

DEPS += \

HEADERS += \
    AntiBacklashController.h \
    EncoderPort.h \
    antibacklashlib.h \
    AntiBacklashLibBuilder.h

SOURCES += \
    AntiBacklashController.cpp \
    AntiBacklashLibBuilder.cpp \
    EncoderPort.cpp

DISTFILES += $$files(*.xml, true) \
    Templates/Models/AntiBacklashLib.AntiBacklashController.xml \
    Templates/Models/AntiBacklashLib.EncoderPort.xml \
    Templates/Recipes/AntiBacklashLib.FC1Node.xml \
    Templates/Recipes/AntiBacklashLib.FC1Port.xml \
    Templates/Recipes/AntiBacklashLib.KublerF5888.xml \
    Templates/Recipes/AntiBacklashLib.KublerF5888_old.xml \
    Templates/Recipes/AntiBacklashLib.VaconFC1.xml

load(cdp)
