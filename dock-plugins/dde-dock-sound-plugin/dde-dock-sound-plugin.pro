
include(../../common.pri)

QT       += core gui widgets dbus

TARGET = dde-dock-sound-plugin
TEMPLATE = lib
CONFIG += plugin c++11
INCLUDEPATH += ../../modules/sound/
DESTDIR = $$_PRO_FILE_PWD_/../

SOURCES += soundplugin.cpp \
    ../../modules/sound/dbus/dbusaudio.cpp \
    ../../modules/sound/dbus/dbusaudiosink.cpp \
    soundicon.cpp \
    mainitem.cpp \
    soundcontent.cpp \
    deviceframe.cpp \
    appframe.cpp \
    ../../modules/sound/dbus/dbusaudiosinkinput.cpp \
    appiconslider.cpp \
    deviceiconslider.cpp \
    iconlabel.cpp \
    volumeslider.cpp

HEADERS += soundplugin.h \
    ../../modules/sound/dbus/dbusaudio.h \
    ../../modules/sound/dbus/dbusaudiosink.h \
    soundicon.h \
    mainitem.h \
    soundcontent.h \
    deviceframe.h \
    appframe.h \
    ../../modules/sound/dbus/dbusaudiosinkinput.h \
    appiconslider.h \
    deviceiconslider.h \
    iconlabel.h \
    volumeslider.h
DISTFILES += dde-dock-sound-plugin.json

target.path = $${PREFIX}/lib/dde-dock/plugins/
INSTALLS += target

PKGCONFIG += gtk+-2.0

RESOURCES += \
    qss.qrc \
    images.qrc

QMAKE_MOC_OPTIONS += -I/usr/include/
