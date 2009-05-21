
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

#
# Copy sample cfg and log.properties files from ../etc
#
TOPDIR=../../../
%.cfg : 
	@ echo Creating $*.cfg from sample configuration at $(TOPDIR)/etc/$*.cfg.sample
	cp -v $(TOPDIR)/etc/$*.cfg.sample ./$*.cfg

%.log.properties: 
	@ echo Creating $*.log.properties from sample configuration at $(TOPDIR)etc/watcher.log.properties.sample
	cp -v $(TOPDIR)etc/watcherd.log.properties.sample ./$*.log.properties



