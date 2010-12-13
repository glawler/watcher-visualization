
message("--- Project General Settings ---")
TEMPLATE = app
TARGET = messageStreamControl
DEPENDPATH += .
INCLUDEPATH += . qtBuildFiles/ui
MOC_DIR = qtBuildFiles/moc
OBJECTS_DIR = qtBuildFiles/obj
RCC_DIR = qtBuildFiles/rcc
UI_DIR = qtBuildFiles/ui

Release {
	DEST_DIR = release
	OBJECTS_DIR = qtBuildFiles/release/obj
	MOC_DIR = qtBuildFiles/release/moc
	RCC_DIR = qtBuildFiles/release/rcc
	UI_DIR = qtBuildFiles/release/ui
}
Debug {
	DESTDIR = debug
	OBJECTS_DIR = qtBuildFiles/debug/obj
	MOC_DIR = qtBuildFiles/debug/moc
	RCC_DIR = qtBuildFiles/debug/rcc
	UI_DIR = qtBuildFiles/debug/ui
}
#
# Input
#
message("--- Project Input Files ---")
HEADERS += \
    QMessageStreamPlaybackWidget.h \
    QWatcherMainWindow.h \
    watcherStreamListDialog.h
SOURCES += \
    main.cpp \
    QMessageStreamPlaybackWidget.cpp \
    QWatcherMainWindow.cpp \
    watcherStreamListDialog.cpp 

#
# Platform specific 
#
macx {
	message("--- Mac OS X specific configuration ---")
    error("messageStreamControl not supported on macOS"); 
} 
else:unix {
	message("--- Unix specific configuration ---")
	DESTDIR = build

    # Tell qmake which pkg-config supported libs to use. 
    CONFIG += link_pkgconfig
    PKGCONFIG += watcher watchermsg

    # If we're using debug and release libs, put them here. 
	Release {
	}
	Debug {
	}
} 
else:win32 {
	message("--- Windows specific configuration ---")
    error("messageStreamControl not supported on Windows."); 
}
#
# UI elements. 
# 
FORMS = \
    ui/messageStreamControl.ui \
    ui/QMessageStreamPlaybackWidget.ui \
    ui/streamlist.ui

RESOURCES= \
    resources\QMessageStreamPlaybackWidget.qrc 
    
