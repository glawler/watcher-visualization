######################################################################
# Automatically generated by qmake (2.01a) Mon Sep 1 16:01:54 2008
######################################################################

TEMPLATE = app
TARGET = watcher
DEPENDPATH += .
INCLUDEPATH += . ./legacyWatcher ../../include ../util ../../include/qwt ../logger ../util
INCLUDEPATH += /usr/include/libxml2 /usr/local/include /usr/local/include/libxml2 /usr/X11R6/include /usr/include/qwt
QT += opengl 
CONFIG += qt x11
OBJECTS_DIR = ./objs
DEFINES += GRAPHICS MODULE_MOBILITY ggdb

# log4cxx issues warnings without this. 
QMAKE_CFLAGS_DEBUG += -fno-strict-aliasing -O0
QMAKE_CXXFLAGS_DEBUG += -fno-strict-aliasing -O0
QMAKE_CFLAGS_RELEASE += -fno-strict-aliasing
QMAKE_CXXFLAGS_RELEASE += -fno-strict-aliasing

LIBS += -L../../lib -L../logger -L../util -L../idsCommunications
LIBS += -L/usr/X11R6/lib -lGL -lGLU -lglut
LIBS += -L/usr/local/lib -lidmef 
LIBS += -lidsCommunications 
LIBS += -lconfig++
LIBS += -llog4cxx
LIBS += -llogger
LIBS += -lwatcherutils
LIBS += -xml2
LIBS += -lqwt

win32 {
    error("No support for windows in watcher")
}

# Input
HEADERS += manetglview.h  \
         singletonConfig.h \
         speedlabel.h \
         watcherScrollingGraphControl.h \
         watcherMainWindow.h \
         graphPlot.h \
         bitmap.h \
         watcherGraphDialog.h \
         legacyWatcher/backgroundImage.h \
         legacyWatcher/skybox.h \
         legacyWatcher/watcherPropertyData.h 

FORMS += watcher.ui 

SOURCES += \
    manetglview.cpp \
    main.cpp \
    singletonConfig.cpp \
    speedlabel.cpp \
    watcherScrollingGraphControl.cpp \
    watcherMainWindow.cpp \
    graphPlot.cpp \
    bitmap.cpp \
    watcherGraphDialog.cpp \
    legacyWatcher/des.cpp \
    legacyWatcher/metric.c \
    legacyWatcher/rng.cc \
    legacyWatcher/node.cpp \
    legacyWatcher/config.c \
    legacyWatcher/idmefPrint.c \
    legacyWatcher/hashtable.c \
    legacyWatcher/floatinglabel.cpp \
    legacyWatcher/legacyWatcher.cpp \
    legacyWatcher/watchermovement.cpp \
    legacyWatcher/watcherGraph.cpp \
    legacyWatcher/mobility.cpp \
    legacyWatcher/watcherGPS.cpp \
    legacyWatcher/graphics.cpp \
    legacyWatcher/apisupport.c \
    legacyWatcher/backgroundImage.cpp \
    legacyWatcher/skybox.cpp \
    legacyWatcher/watcherPropertyData.cpp