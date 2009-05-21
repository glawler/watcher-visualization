# melkins - compilation for Fedora 10
-include ../Makefile.local

TOPDIR=../..

SUBDIRS=. test

# "Fixes" warning in log4cxx about type punning. 
CPPFLAGS += -fno-strict-aliasing

CPPFLAGS += -I..
CPPFLAGS += -I../util
CPPFLAGS += -I../logger
CPPFLAGS += -I../sqlite_wrapper
CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += $(BOOST_CPPFLAGS)

LDFLAGS += -L$(TOPDIR)/lib
LDFLAGS += $(BOOST_LDFLAGS)

# watcher libs
LDADD = libfeeder.a \
		libwatcherdAPI.a \
    ../sqlite_wrapper/libsqlite_wrapper.a \
    ../libwatcher/libwatcher.a \
    ../util/libwatcherutils.a \
    ../logger/liblogger.a

# system libraries
LIBS+=-lconfig++
LIBS+=-llog4cxx
LIBS+=-lsqlite3
LIBS+=$(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# 
# .cfg and .log.properties build rules are in top level Makefile.am
#
BUILT_SOURCES= \
	connectivity2dot.cfg \
	connectivity2dot.log.properties \
	watcherd.cfg \
	watcherd.log.properties \
	testCC.cfg \
	testCC.log.properties \
	messageStream2Text.cfg \
	messageStream2Text.log.properties \
	routeFeeder.cfg \
	routeFeeder.log.properties

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
	singletonConfig.cpp \
	watcherGraph.cpp

bin_PROGRAMS= \
	watcherd \
	gpsMessageTest \
	labelMessageTest \
	edgeMessageTest \
	colorMessageTest \
	dataRequestMessageTest \
	connectivityMessageTest \
	testCC \
	messageStream2Text \
	connectivity2dot

connectivity2dot_LFLAGS=$(LFLAGS) -L. -lwatcherdAPI

watcherd_SOURCES=watcherdMain.cpp watcherd.cpp server.cpp serverConnection.cpp \
		 serverMessageHandler.cpp writeDBMessageHandler.cpp \
		 writeDBMessageHandler.h database.h database.cpp \
		 sqliteDatabase.h sqliteDatabase.cpp watcherdConfig.h \
		 watcherdConfig.cpp replayState.h replayState.cpp

ctags:
	ctags -R $(TOPDIR)

#
# Copy sample cfg and log.properties files from ../etc
#
%.cfg : 
	@ echo Creating $*.cfg from sample configuration at ../etc/$*.cfg.sample
	cp -v ../etc/$*.cfg.sample ./$*.cfg

%.log.properties: 
	@ echo Creating $*.log.properties from sample configuration at ../etc/watcher.log.properties.sample
	cp -v ../etc/watcherd.log.properties.sample ./$*.log.properties

