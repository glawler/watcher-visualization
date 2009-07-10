include $(srcdir)/../Makefile.clients

CPPFLAGS += -I$(DELTA_ROOT)/inc # for Delta3D
LDFLAGS += -L$(DELTA_ROOT)/lib # for Delta3D
LIBS+=-losgDB -ldtABC -ldtCore -ldtTerrain -ldtActors -ldtGame # for Delta3D

BUILT_SOURCES=watcher3D.cfg watcher3D.log.properties 

lib_LTLIBRARIES=\
	libActors.la \
	libWatcher3D.la

libActors_la_SOURCES=libActors.cpp nodeActor.cpp nodeActorProxy.cpp edgeActor.cpp edgeActorProxy.cpp
libActors_la_LIBADD = ../../logger/liblogger.la
libWatcher3D_la_SOURCES=libWatcher3D.cpp

# watcher3D - non-standard game executable.
bin_PROGRAMS=watcher3D
watcher3D_SOURCES=main.cpp
