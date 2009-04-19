# GTL - experimental automatic use of multiprocessor systems to speed up make
# May break things, don't know.
#PROCCOUNT=$(shell cat /proc/cpuinfo | grep -c ^processor)
#ifneq (${PROCCOUNT}, 0)
#      MAKEFLAGS+=-j $(PROCCOUNT)
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

# GTL - TBD remove objs if not needed
libfeeder_a_SOURCES= \
	clientConnection.cpp \
	client.cpp \
	connection.cpp \
	dataMarshaller.cpp \
	messageFactory.cpp \
	messageHandler.cpp \
	singletonConfig.cpp \
	dataMarshaller.cpp \
	messageFactory.cpp \
	messageHandler.cpp \
	messageStream.cpp \
	messageStreamFilter.cpp \
	feederAPIMessageHandler.cpp \
	watcherdAPIMessageHandler.cpp \
	singletonConfig.cpp 

libwatcherdAPI_a_SOURCES= \
	clientConnection.cpp \
	client.cpp \
	feederAPIMessageHandler.cpp \
	watcherdAPIMessageHandler.cpp \
	connection.cpp \
	dataMarshaller.cpp \
	messageFactory.cpp \
	messageHandler.cpp \
	singletonConfig.cpp \
	dataMarshaller.cpp \
	messageFactory.cpp \
	messageHandler.cpp \
	messageStream.cpp \
	messageStreamFilter.cpp \
	singletonConfig.cpp 

bin_PROGRAMS= \
	watcherd \
	gpsMessageTest \
	labelMessageTest \
	edgeMessageTest \
	colorMessageTest \
	dataRequestMessageTest \
	testCC \
	messageStream2Text

watcherd_SOURCES=watcherdMain.cpp watcherd.cpp server.cpp serverConnection.cpp serverMessageHandler.cpp

ctags:
	ctags -R $(TOPDIR)

watcherd.cfg: 
	@ echo Creating watcherd.cfg from sample configuration at ../../etc/watcherd.cfg.sample
	cp ../../etc/watcherd.cfg.sample ./watcherd.cfg

watcherd.log.properties: 
	@ echo Creating log.properties from sample configuration in ../../etc/watcherd.log.properties.sample
	cp ../../etc/watcherd.log.properties.sample ./watcherd.log.properties
