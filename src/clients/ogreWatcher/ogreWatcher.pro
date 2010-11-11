
message("--- Project General Settings ---")
TEMPLATE = app
TARGET = ogreWatcher
DEPENDPATH += .
INCLUDEPATH += .
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
    QOgreWindowWidget.h \
    QOgreWidget.h 
SOURCES += \
    main.cpp \
    QOgreWindowWidget.cpp \
    QOgreWidget.cpp \
    qtBuildFiles/ui/ui_ogreWatcher.h \
    QSdkCameraMan.h 
#
# Platform specific 
#
macx {
	message("--- Mac OS X specific configuration ---")
	CONFIG += x86 ppc
	LIBS += -framework Ogre -framework AGL
	INCLUDEPATH += /Library/Frameworks/Ogre.framework/Headers
	
	# create link to data dir in the build dir
	system(mkdir build)
	system(ln -fs ../Data build/Data)
} 
else:unix {
	message("--- Unix specific configuration ---")
	DESTDIR = build

    # Tell qmake which pkg-config supported libs to use. 
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE OIS

    # If we're using debug and release libs, put them here. 
	Release {
	}
	Debug {
	}
} 
else:win32 {
	message("--- Windows specific configuration ---")
	TEMPLATE = vcapp
	CONFIG += console
	INCLUDEPATH += $$(OGRE_HOME)\include
	LIBPATH += $$(OGRE_HOME)\lib
	
	Release {
		LIBS *= -lOgreMain
	}
	Debug {
		LIBS *= -lOgreMain_d
	}
}
#
# UI elements. 
# 
FORMS = \
    ui/ogreWatcher.ui
