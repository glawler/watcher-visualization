include $(srcdir)/../Makefile.clients

CPPFLAGS += -I$(DELTA_ROOT)/inc # for Delta3D
LDFLAGS += -L$(DELTA_ROOT)/lib # for Delta3D
LIBS+=-losgDB -ldtABC -ldtCore -ldtTerrain -ldtActors -ldtGame # for Delta3D
LDADD+=./libActors.la 

BUILT_SOURCES=watcher3D.cfg watcher3D.log.properties 

lib_LTLIBRARIES=libActors.la

libActors_la_SOURCES=libActors.cpp nodeActor.cpp nodeActorProxy.cpp edgeActor.cpp edgeActorProxy.cpp

# watcher3D - non-standard game executable.
bin_PROGRAMS=watcher3D
watcher3D_SOURCES=main.cpp watcher3D.cpp

