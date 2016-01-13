
include(../../common.pri)

QT       += core gui widgets dbus
TEMPLATE = lib
CONFIG += plugin c++11
LIBS += -ldui -L../../widgets -lwidgets
INCLUDEPATH += ../../frame/ ../../widgets
INCLUDEPATH += /usr/include/libdui/
DESTDIR         = $$_PRO_FILE_PWD_/../

SOURCES += personalization.cpp \
    dbus/appearancedaemon_interface.cpp \
    dbusworker.cpp
HEADERS += personalization.h \
    dbus/appearancedaemon_interface.h \
    dbusworker.h \
    persionalizationfwd.h
DISTFILES += \
    personalization.json \
    dbus/appearancedaemon.xml

TARGET          = $$qtLibraryTarget(personalization)

target.path = $${PREFIX}/lib/dde-control-center/modules/
INSTALLS += target

RESOURCES +=
