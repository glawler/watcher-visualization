SUBDIRS=. test

# "Fixes" warning in log4cxx about type punning. 
CXXFLAGS += -fno-strict-aliasing

CPPFLAGS += -I../../include 		# lob4cxx, libconfig++
CPPFLAGS += -I../util				# watcherutils
CPPFLAGS += -I../logger				# logger
CPPFLAGS += -I..
CPPFLAGS += @BOOST_CPPFLAGS@

noinst_LIBRARIES=libwatcher.a

libwatcher_a_SOURCES= \
		      client.h client.cpp \
		      clientConnection.h clientConnection.cpp \
	colorMessage.cpp \
	connection.cpp \
	connection_fwd.h \
	connection.h \
	connectivityMessage.cpp \
	dataMarshaller.h dataMarshaller.cpp \
	dataRequestMessage.cpp \
	displayInfo.cpp \
	edgeDisplayInfo.cpp \
	edgeMessage.cpp \
	gpsMessage.cpp \
	labelDisplayInfo.cpp \
	labelMessage.cpp \
	message.cpp \
	messageHandler.h messageHandlerFwd.h messageHandler.cpp \
	messageStatus.cpp \
	messageStream.h messageStream.cpp \
	messageStreamFilter.h messageStreamFilter.cpp \
	messageTypesAndVersions.cpp \
	nodeDisplayInfo.cpp \
	nodeStatusMessage.cpp \
	seekWatcherMessage.cpp \
	speedWatcherMessage.cpp \
	startWatcherMessage.cpp \
	stopWatcherMessage.cpp \
	testMessage.cpp \
	watcherColors.cpp \
	watcherGlobalFunctions.cpp \
	watcherGraph.cpp \
	watcherGraphEdge.cpp \
	watcherGraphNode.cpp \
	watcherRegion.h watcherRegion.cpp \
	watcherdAPIMessageHandler.h watcherdAPIMessageHandler.cpp
