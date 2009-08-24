SUBDIRS=. test

include $(top_srcdir)/Makefile.top

lib_LIBRARIES=libwatcher.a
libwatcher_a_includedir=$(includedir)/libwatcher
libwatcher_a_include_HEADERS=\
	client.h \
	message_fwd.h \
	watcherTypes.h \
	watcherGlobalFunctions.h


lib_LTLIBRARIES=libwatchermsg.la
libwatchermsg_la_includedir=$(includedir)/libwatcher
libwatchermsg_la_include_HEADERS=\
	message.h \
	colorMessage.h \
	connectivityMessage.h \
	dataPointMessage.h \
	edgeMessage.h \
	gpsMessage.h \
	labelMessage.h \
	messageTypesAndVersions.h \
	nodeStatusMessage.h \
	watcherColors.h
	
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = watcher.pc watchermsg.pc

libwatchermsg_la_SOURCES=message.h message.cpp \
	colorMessage.cpp \
	connectivityMessage.cpp \
	dataPointMessage.cpp \
	edgeMessage.cpp \
	gpsMessage.cpp \
	labelMessage.cpp \
	messageTypesAndVersions.cpp \
	nodeStatusMessage.cpp \
	playbackTimeRange.cpp \
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
	watcherdAPIMessageHandler.h watcherdAPIMessageHandler.cpp \
	watcherTypes.cpp
