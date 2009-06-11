include $(srcdir)/../Makefile.clients

CPPFLAGS += -I$(DELTA_ROOT)/inc # for Delta3D
LDFLAGS += -L$(DELTA_ROOT)/lib # for Delta3D
LIBS+=-losgDB -ldtABC -ldtCore -ldtTerrain -ldtActors -ldtGame # for Delta3D
LDADD+=./libNodeActor.la 
# LDADD+=./libEdgeActor.la

BUILT_SOURCES=watcher3D.cfg watcher3D.log.properties 

lib_LTLIBRARIES=libEdgeActor.la libNodeActor.la # libWatcher3DGameEntryPoint.la

# EdgeActor - represents an edge.
libEdgeActor_la_SOURCES=libEdgeActor.cpp edgeActor.cpp edgeActorProxy.cpp
# NodeActor - represents a node.
libNodeActor_la_SOURCES=libNodeActor.cpp nodeActor.cpp nodeActorProxy.cpp

# Watcher3DGameEntryPoint - standard game executable. 
# libWatcher3DGameEntryPoint_la_SOURCES=watcher3DGameEntryPoint.cpp

# watcher3D - non-standard game executable.
bin_PROGRAMS=watcher3D
watcher3D_SOURCES=main.cpp watcher3D.cpp

