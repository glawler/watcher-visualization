
TOPDIR=../../..
SRCDIR=$(TOPDIR)/src

# "Fixes" warning in log4cxx about type punning. 
CPPFLAGS += -fno-strict-aliasing

CPPFLAGS += -I$(SRCDIR)
CPPFLAGS += -I$(SRCDIR)/watcherd
CPPFLAGS += -I$(SRCDIR)/util
CPPFLAGS += -I$(SRCDIR)/logger
CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += $(BOOST_CPPFLAGS)
CPPFLAGS += -I$(DELTA_ROOT)/inc # for Delta3D

LDFLAGS+=-L$(SRCDIR)/watcherd
LDFLAGS+=-L$(SRCDIR)/libwatcher
LDFLAGS+=-L$(SRCDIR)/util
LDFLAGS+=-L$(SRCDIR)/logger
LDFLAGS+=-L$(TOPDIR)/lib
LDFLAGS += $(BOOST_LDFLAGS)
LDFLAGS += -L$(DELTA_ROOT)/lib # for Delta3D

LIBS+=-lconfig++
LIBS+=-llogger
LIBS+=-llog4cxx
LIBS+=-lwatcherutils
LIBS+=-lwatcherd
LIBS+=-lwatcher
LIBS+=$(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB)
LIBS+=-losgDB -ldtABC -ldtCore -ldtTerrain -ldtActors -ldtGame # for Delta3D

BUILT_SOURCES=watcher3D.cfg watcher3D.log.properties 

bin_PROGRAMS=watcher3D

watcher3D_SOURCES=main.cpp watcher3D.cpp

noinst_LIBRARIES=libEdgeActor.so

libEdgeActor_so_SOURCES=libEdgeActor.cpp edgeActor.cpp edgeActorProxy.cpp

ctags:
	ctags -R $(SRCDIR)

#
# Copy sample cfg and log.properties files from ../etc
#
%.cfg : 
	@ echo Creating $*.cfg from sample configuration at $(TOPDIR)/etc/$*.cfg.sample
	cp -v $(TOPDIR)/etc/$*.cfg.sample ./$*.cfg

%.log.properties: 
	@ echo Creating $*.log.properties from sample configuration at $(TOPDIR)/etc/watcher.log.properties.sample
	cp -v $(TOPDIR)/etc/watcherd.log.properties.sample ./$*.log.properties
	@ echo This is probably a bad idea - attempting to set the correct output log file in log.properties.
	@ perl -pi -e 's/watcherd.log/$*.log/g' ./$*.log.properties




