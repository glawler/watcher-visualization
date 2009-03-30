# melkins - compilation for Fedora 10
-include ../Makefile.local

TOPDIR=../..

CPPFLAGS += -I..
CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += @BOOST_CPPFLAGS@

LDFLAGS+=-L../libwatcher
LDFLAGS+=-L$(TOPDIR)/lib
LDFLAGS += @BOOST_LDFLAGS@

LIBS+=-lwatcher
LIBS+=-lconfig++
LIBS+=-llogger
LIBS+=-llog4cxx
LIBS+=-lwatcherutils
LIBS+=$(BOOST_BASE_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB)

#all: $(BINS) watcherd.cfg watcherd.log.properties 
#MAINS=gpsMessageTest.cpp testCC.cpp labelMessageTest.cpp edgeMessageTest.cpp colorMessageTest.cpp dataRequestMessageTest.cpp
bin_PROGRAMS=watcherd gpsMessageTest labelMessageTest edgeMessageTest colorMessageTest dataRequestMessageTest testCC
watcherd_SOURCES= messageFactory.cpp singletonConfig.cpp \
	 serverConnection.cpp dataMarshaller.cpp server.cpp \
	 clientConnection.cpp messageHandler.cpp messageStatusHandler.cpp \
	 gpsMessageHandler.cpp messageHandlerFactory.cpp client.cpp \
	 labelMessageHandler.cpp  edgeMessageHandler.cpp \
	 colorMessageHandler.cpp watcherd.cpp dataRequestMessageHandler.cpp \
	 watcherdClientConnection.cpp

# GTL - experimental automatic use of multiprocessor systems to speed up make
# May break things, don't know.
#PROCCOUNT=$(shell cat /proc/cpuinfo | grep -c ^processor)
#ifneq (${PROCCOUNT}, 0)
#	MAKEFLAGS+=-j $(PROCCOUNT)
#endif

watcherd.cfg: 
	@ echo Creating watcherd.cfg from sample configuration at ../../etc/watcherd.cfg.sample
	cp ../../etc/watcherd.cfg.sample ./watcherd.cfg

watcherd.log.properties: 
	@ echo Creating log.properties from sample configuration in ../../etc/watcherd.log.properties.sample
	cp ../../etc/watcherd.log.properties.sample ./watcherd.log.properties
