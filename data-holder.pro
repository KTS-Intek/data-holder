QT += core network
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = data-holder

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION = 0.0.1

#DEFINES += APPLCTN_NAME=\\\"quick-collect\\\" it is only for GUI
DEFINES += "MYAPPNAME=\"\\\"data-holder\\\"\""
DEFINES += "MYAPPOWNER=\"\\\"KTS-Intek Ltd\\\"\""
DEFINES += "MYAPPOWNERSITE=\"\\\"http://kts-intek.com\\\"\""

include(../../../Matilda-units/ipc/sharedmemory/sharedmemory.pri)
#include(../../Matilda-units/ipc/localsockets/localsockets.pri)

linux:{
    target.path = /opt/matilda/bin
    INSTALLS += target
}


SOURCES += \
        data-holder-src/dataholderlocalserver.cpp \
        data-holder-src/dataholderlocalsocket.cpp \
        data-holder-src/dataholdermanager.cpp \
        data-holder-src/dataholdersharedmemoryobject.cpp \
        data-holder-src/dataholdersharedobject.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    data-holder-src/dataholderlocalserver.h \
    data-holder-src/dataholderlocalservercommands.h \
    data-holder-src/dataholderlocalsocket.h \
    data-holder-src/dataholdermanager.h \
    data-holder-src/dataholdersharedmemoryobject.h \
    data-holder-src/dataholdersharedobject.h \
    data-holder-src/dataholdertypes.h
