
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

BUILT_SOURCES=connectivityFeeder.cfg connectivityFeeder.log.properties 

bin_PROGRAMS=connectivityFeeder

connectivityFeeder_SOURCES=connectivityFeeder.cpp

ctags:
	ctags -R $(TOPDIR)

connectivityFeeder.cfg: 
	@ echo Creating connectivityFeeder.cfg from sample configuration at $(TOPDIR)/etc/connectivityFeeder.cfg.sample
	cp $(TOPDIR)/etc/connectivityFeeder.cfg.sample ./connectivityFeeder.cfg

connectivityFeeder.log.properties: 
	@ echo Creating log.properties from sample configuration in $(TOPDIR)/etc/connectivityFeeder.log.properties.sample
	cp $(TOPDIR)/etc/connectivityFeeder.log.properties.sample ./connectivityFeeder.log.properties




