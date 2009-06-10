SUBDIRS=. test

include $(top_srcdir)/Makefile.top

noinst_LIBRARIES=libwatcher.a
lib_LTLIBRARIES=libwatchermsg.la

libwatchermsg_la_SOURCES=message.h message.cpp \
	colorMessage.cpp \
	connectivityMessage.cpp \
	dataRequestMessage.cpp \
	edgeMessage.cpp \
	gpsMessage.cpp \
	labelMessage.cpp \
	messageTypesAndVersions.cpp \
	nodeStatusMessage.cpp \
	seekWatcherMessage.cpp \
	speedWatcherMessage.cpp \
	startWatcherMessage.cpp \
	stopWatcherMessage.cpp \
	testMessage.cpp \
	watcherColors.cpp

libwatcher_a_SOURCES= \
		      client.h client.cpp \
		      clientConnection.h clientConnection.cpp \
	connection.cpp \
	connection_fwd.h \
	connection.h \
	dataMarshaller.h dataMarshaller.cpp \
	displayInfo.cpp \
	edgeDisplayInfo.cpp \
	labelDisplayInfo.cpp \
	messageHandler.h messageHandlerFwd.h messageHandler.cpp \
	messageStatus.cpp \
	messageStream.h messageStream.cpp \
	messageStreamFilter.h messageStreamFilter.cpp \
	nodeDisplayInfo.cpp \
	watcherGlobalFunctions.cpp \
	watcherGraph.cpp \
	watcherGraphEdge.cpp \
	watcherGraphNode.cpp \
	watcherRegion.h watcherRegion.cpp \
	watcherdAPIMessageHandler.h watcherdAPIMessageHandler.cpp
