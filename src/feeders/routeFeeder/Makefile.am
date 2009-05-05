
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

LDFLAGS+=-L$(SRCDIR)/watcherd
LDFLAGS+=-L$(SRCDIR)/libwatcher
LDFLAGS+=-L$(SRCDIR)/util
LDFLAGS+=-L$(SRCDIR)/logger
LDFLAGS+=-L$(TOPDIR)/lib
LDFLAGS += $(BOOST_LDFLAGS)

LIBS+=-lconfig++
LIBS+=-llogger
LIBS+=-llog4cxx
LIBS+=-lwatcherutils
LIBS+=-lfeeder
LIBS+=-lwatcher
LIBS+=$(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

BUILT_SOURCES=routeFeeder.cfg routeFeeder.log.properties 

bin_PROGRAMS=routeFeeder

routeFeeder_SOURCES=routeFeeder.cpp

ctags:
	ctags -R $(TOPDIR)

routeFeeder.cfg: 
	@ echo Creating routeFeeder.cfg from sample configuration at $(TOPDIR)/etc/routeFeeder.cfg.sample
	cp $(TOPDIR)/etc/routeFeeder.cfg.sample ./routeFeeder.cfg

routeFeeder.log.properties: 
	@ echo Creating log.properties from sample configuration in $(TOPDIR)/etc/routeFeeder.log.properties.sample
	cp $(TOPDIR)/etc/routeFeeder.log.properties.sample ./routeFeeder.log.properties




