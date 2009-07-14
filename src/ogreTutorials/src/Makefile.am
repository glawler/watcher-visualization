
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
	tutorial_2 \
	tutorial_3 \
	tutorial_4 \
	tutorial_5 \
	tutorial_6 \
	tutorial_7 \
	tutorial_8 

SampleApp_SOURCES=SampleApp.cpp
tutorial_1_SOURCES=tutorial_1.cpp
tutorial_2_SOURCES=tutorial_2.cpp
tutorial_3_SOURCES=tutorial_3.cpp
tutorial_4_SOURCES=tutorial_4.cpp
tutorial_5_SOURCES=tutorial_5.cpp
tutorial_6_SOURCES=tutorial_6.cpp
tutorial_7_SOURCES=tutorial_7.cpp
tutorial_8_SOURCES=tutorial_8.cpp


