include $(srcdir)/../Makefile.clients

CPPFLAGS += -I$(DELTA_ROOT)/inc # for Delta3D
LDFLAGS += -L$(DELTA_ROOT)/lib # for Delta3D
LIBS+=-losgDB -ldtABC -ldtCore -ldtTerrain -ldtActors -ldtGame # for Delta3D
LDADD+=./libNodeActor.la

lib_LTLIBRARIES=libEdgeActor.la libNodeActor.la
libEdgeActor_la_SOURCES=libEdgeActor.cpp edgeActor.cpp edgeActorProxy.cpp
libNodeActor_la_SOURCES=libNodeActor.cpp nodeActor.cpp nodeActorProxy.cpp

BUILT_SOURCES=watcher3D.cfg watcher3D.log.properties 

bin_PROGRAMS=watcher3D
watcher3D_SOURCES=main.cpp watcher3D.cpp

