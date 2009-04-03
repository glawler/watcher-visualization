# GTL - experimental automatic use of multiprocessor systems to speed up make
# May break things, don't know.
#PROCCOUNT=$(shell cat /proc/cpuinfo | grep -c ^processor)
#ifneq (${PROCCOUNT}, 0)
#	MAKEFLAGS+=-j $(PROCCOUNT)
#endif

# melkins - compilation for Fedora 10
-include ../Makefile.local

TOPDIR=../..

SUBDIRS=. test

# "Fixes" warning in log4cxx about type punning. 
CPPFLAGS += -fno-strict-aliasing

CPPFLAGS += -I..
CPPFLAGS += -I../util
CPPFLAGS += -I../logger
CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += $(BOOST_CPPFLAGS)

LDFLAGS+=-L../libwatcher
LDFLAGS+=-L../util
LDFLAGS+=-L../logger
LDFLAGS+=-L$(TOPDIR)/lib
LDFLAGS += $(BOOST_LDFLAGS)

LDADD = libfeeder.a

LIBS+=-lwatcher
LIBS+=-lconfig++
LIBS+=-llogger
LIBS+=-llog4cxx
LIBS+=-lwatcherutils
LIBS+=$(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)


BUILT_SOURCES=watcherd.cfg watcherd.log.properties 

noinst_LIBRARIES=libfeeder.a libwatcherdAPI.a
libfeeder_a_SOURCES= \
	messageFactory.cpp singletonConfig.cpp \
	 serverConnection.cpp dataMarshaller.cpp server.cpp \
	 clientConnection.cpp messageHandler.cpp messageStatusHandler.cpp \
	 gpsMessageHandler.cpp messageHandlerFactory.cpp client.cpp \
	 labelMessageHandler.cpp  edgeMessageHandler.cpp testMessageHandler.cpp \
	 colorMessageHandler.cpp watcherd.cpp dataRequestMessageHandler.cpp \
	 watcherdClientConnection.cpp 
libwatcherdAPI_a_SOURCES= \
	 messageStream.cpp clientConnection.cpp messageStreamFilter.cpp watcherdClientConnection.cpp client.cpp

bin_PROGRAMS=watcherd gpsMessageTest labelMessageTest edgeMessageTest colorMessageTest dataRequestMessageTest testCC
watcherd_SOURCES=watcherdMain.cpp

ctags:
	ctags -R .. 

watcherd.cfg: 
	@ echo Creating watcherd.cfg from sample configuration at ../../etc/watcherd.cfg.sample
	cp ../../etc/watcherd.cfg.sample ./watcherd.cfg

watcherd.log.properties: 
	@ echo Creating log.properties from sample configuration in ../../etc/watcherd.log.properties.sample
	cp ../../etc/watcherd.log.properties.sample ./watcherd.log.properties
