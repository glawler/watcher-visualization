
# OGRE
CPPFLAGS=-I$(top_srcdir)/include
CXXFLAGS=$(OGRE_CFLAGS) $(OIS_CFLAGS)
LDADD=$(OGRE_LIBS) $(OIS_LIBS)

# CEGUI - for tutorial_6 and beyond
CPPFLAGS+=-I/usr/include/CEGUI
CXXFLAGS+=-I/usr/include/CEGUI
LDADD+=-lCEGUIBase -lCEGUIOgreRenderer

bin_PROGRAMS= \
	SampleApp \
	tutorial_1 \
	camerasLightsShadows \
	skyFogRootObject \
	frameListenersUnbufferedInput \
	bufferedInput \
	ogreStartupSequence \
	ceguiAndOgre \
	multipleSceneManagers \
	walkingBetweenPointsBasicQuaternions

SampleApp_SOURCES=SampleApp.cpp
tutorial_1_SOURCES=tutorial_1.cpp
camerasLightsShadows_SOURCES=camerasLightsShadows.cpp
skyFogRootObject_SOURCES=skyFogRootObject.cpp
frameListenersUnbufferedInput_SOURCES=frameListenersUnbufferedInput.cpp
bufferedInput_SOURCES=bufferedInput.cpp
ogreStartupSequence_SOURCES=ogreStartupSequence.cpp
ceguiAndOgre_SOURCES=ceguiAndOgre.cpp
multipleSceneManagers_SOURCES=multipleSceneManagers.cpp
walkingBetweenPointsBasicQuaternions_SOURCES=walkingBetweenPointsBasicQuaternions.cpp


