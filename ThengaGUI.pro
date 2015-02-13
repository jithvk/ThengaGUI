#-------------------------------------------------
#
# Project created by QtCreator 2011-02-17T19:31:55
#
#-------------------------------------------------

QT       += core gui network opengl widgets

#version depending on the date
VERSION = 0.07.23

TARGET = ThengaGUI
TEMPLATE = app
INCLUDEPATH = inc/
INCLUDEPATH += ui/inc



INCLUDEPATH+=../A2DB/adb
INCLUDEPATH+=../A2DB/include
INCLUDEPATH+=../QTermWidget/inc
#LIBS+= -L../A2DB/debug -la2db -lpthread
LIBS+= -L../A2DB/ -la2db -lpthread \
        -L../QTermWidget/ -lqAdbTermWidget

win32 {
RC_FILE = FatTest.rc

LIBS += -lws2_32
}



QMAKE_CFLAGS_RELEASE    = -Wall -Wno-unused-parameter
QMAKE_CFLAGS_DEBUG      = -Wall -Wno-unused-parameter



DEFINES += APP_VERSION=\\\"$$VERSION\\\"

SOURCES += ui/src/main.cpp\
    src/detailedlog.cpp \
    ui/src/settingsdialog.cpp \
    ui/src/generalsettingspage.cpp \
    ui/src/imageviewergl.cpp \
    ui/src/imageview.cpp \
    src/direntry.cpp \
    src/filesystemmodel.cpp \
    src/fileobject.cpp \
    src/fileobjecthandler.cpp \
    src/filestorage.cpp \
    src/adbhandler.cpp \
    ui/src/mainwindow.cpp \
    ui/src/adbshell.cpp \
    ui/src/abstractmdichild.cpp \
    ui/src/welcome.cpp \
    ui/src/filebrowser.cpp \
    ui/src/adbfilebrowser.cpp \
    src/filesystemfilter.cpp \
    src/filelistview.cpp \
    ui/src/cpugraphview.cpp \
    src/adb_file_sync.cpp \
    src/adbsynchelper.cpp \
    src/fileiconprovider.cpp



HEADERS  += inc/detailedlog.h \
    ui/inc/settingsdialog.h \
    ui/inc/generalsettingspage.h \
    ui/inc/imageview.h \
    ui/inc/imageviewergl.h \
    inc/direntry.h \
    inc/filesystemmodel.h \
    inc/fileobject.h \
    inc/fileobjecthandler.h \
    inc/filestorage.h \
    inc/adbhandler.h \
    ui/inc/mainwindow.h \
    ui/inc/adbshell.h \
    ui/inc/abstractmdichild.h \
    ui/inc/welcome.h \
    ui/inc/filebrowser.h \
    ui/inc/adbfilebrowser.h \
    inc/filesystemfilter.h \
    inc/filelistview.h \
    ui/inc/cpugraphview.h \
    inc/adbsynchelper.h \
    inc/fileiconprovider.h




FORMS    += ui/forms/mainwindow.ui \
    ui/forms/settingsdialog.ui \
    ui/forms/generalsettingspage.ui \
    ui/forms/imagebrowser.ui \
    ui/forms/imageview.ui \
    ui/forms/welcome.ui \
    ui/forms/adbfilebrowser.ui \
    ui/forms/cpugraphview.ui

RESOURCES += \
    Resources1.qrc

OTHER_FILES += \
    FatTest.rc
