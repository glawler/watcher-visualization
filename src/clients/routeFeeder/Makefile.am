
TOPDIR=../../..
SRCDIR=$(TOPDIR)/src

# "Fixes" warning in log4cxx about type punning. 
CPPFLAGS += -fno-strict-aliasing

CPPFLAGS += -I$(SRCDIR)
CPPFLAGS += -I$(SRCDIR)/util
CPPFLAGS += -I$(SRCDIR)/logger
CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += $(BOOST_CPPFLAGS)

LDFLAGS+=-L$(TOPDIR)/lib
LDFLAGS += $(BOOST_LDFLAGS)

LIBS+=-lconfig++
LIBS+=-llog4cxx
LIBS+=$(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

LDADD = $(SRCDIR)/libwatcher/libwatcher.a 
LDADD += $(SRCDIR)/libwatcher/libwatchermsg.la 
LDADD += $(SRCDIR)/sqlite_wrapper/libsqlite_wrapper.a
LDADD += $(SRCDIR)/util/libwatcherutils.a 
LDADD += $(SRCDIR)/logger/liblogger.a

BUILT_SOURCES=routeFeeder.cfg routeFeeder.log.properties 

bin_PROGRAMS=routeFeeder

routeFeeder_SOURCES=routeFeeder.cpp

ctags:
	ctags -R $(TOPDIR)

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




