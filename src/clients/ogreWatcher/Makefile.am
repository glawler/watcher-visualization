
# watcher
include $(srcdir)/../Makefile.clients
CPPFLAGS+=-I$(top_srcdir)/logger
CPPFLAGS+=-I$(top_srcdir)/util
CPPFLAGS+=-I$(top_srcdir)/libwatcher

# OGRE
CXXFLAGS+=$(OGRE_CFLAGS) $(OIS_CFLAGS) $(CEGUI_CFLAGS) $(CEGUI-OGRE_CFLAGS)
LDADD+=$(OGRE_LIBS) $(OIS_LIBS) $(CEGUI_LIBS) $(CEGUI-OGRE_LIBS) -lCEGUIOgreRenderer

BUILT_SOURCES=resources.cfg plugins.cfg ogreWatcher.cfg ogreWatcher.log.properties

bin_PROGRAMS= \
	ogreWatcher

ogreWatcher_SOURCES= \
	ogreWatcherMain.cpp \
	ogreWatcher.cpp \
    ogreApplication.cpp \
	ogreFrameListener.cpp \
	ogreWatcherApplication.cpp \
	node.cpp \
	edge.cpp \
	watcherMessageFrameListener.cpp \
	edge.h \
	node.h \
	ogreApplication.h \
	ogreFrameListener.h \
	ogreWatcherApplication.h \
	watcherMessageFrameListener.h


resources.cfg : 
	@ echo Creating resources.cfg from sample configuration at $(TOPDIR)/etc/ogre.resources.cfg.sample
	cp -v $(TOPDIR)/etc/ogre.resources.cfg.sample ./resources.cfg 

plugins.cfg : 
	@ echo Creating plugins.cfg from sample configuration at $(TOPDIR)/etc/ogre.plugins.cfg.sample
	cp -v $(TOPDIR)/etc/ogre.plugins.cfg.sample ./plugins.cfg 

ogreWatcher.log.properties:
	cp -v $(TOPDIR)/etc/ogreWatcher.log.properties.sample ogreWatcher.log.properties


