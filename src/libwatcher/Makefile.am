SUBDIRS=. test

include $(top_srcdir)/Makefile.top

lib_LIBRARIES=libwatcher.a
libwatcher_a_includedir=$(includedir)/libwatcher
libwatcher_a_include_HEADERS=\
	client.h \
	connection_fwd.h \
	message_fwd.h \
	messageHandler.h \
	sendMessageHandler.h \
	messageStream.h \
	messageStreamReactor.h \
	watcherdAPIMessageHandler.h \
	watcherTypes.h \
	watcherGlobalFunctions.h \
	watcherColors.h \
	colors.h \
	message.h \
	colorMessage.h \
	connectivityMessage.h \
	dataPointMessage.h \
	edgeMessage.h \
	gpsMessage.h \
	labelMessage.h \
	messageTypesAndVersions.h \
	messageStreamFilter.h \
	messageStreamFilterMessage.h \
	seekWatcherMessage.h \
	playbackTimeRange.h \
	listStreamsMessage.h \
	speedWatcherMessage.h \
	streamDescriptionMessage.h \
	nodePropertiesMessage.h \
	nodeStatusMessage.h \
	watcherMessageFwd.h 
	
libwatcher_a_SOURCES=\
	message.h message.cpp \
	colorMessage.cpp colorMessage.h \
	connectivityMessage.cpp connectivityMessage.h \
	dataPointMessage.cpp dataPointMessage.h \
	edgeMessage.cpp edgeMessage.h \
	gpsMessage.cpp gpsMessage.h \
	labelMessage.cpp labelMessage.h \
	listStreamsMessage.cpp listStreamsMessage.h \
	messageTypesAndVersions.cpp messageTypesAndVersions.h \
	messageStreamFilterMessage.h messageStreamFilterMessage.cpp \
	nodePropertiesMessage.cpp nodePropertiesMessage.h \
	nodeStatusMessage.cpp nodeStatusMessage.h \
	playbackTimeRange.cpp playbackTimeRange.h \
	seekWatcherMessage.cpp seekWatcherMessage.h \
	speedWatcherMessage.cpp speedWatcherMessage.h \
	startWatcherMessage.cpp startWatcherMessage.h \
	stopWatcherMessage.cpp stopWatcherMessage.h \
	streamDescriptionMessage.cpp streamDescriptionMessage.h \
	subscribeStreamMessage.cpp subscribeStreamMessage.h \
	client.h client.cpp \
	clientConnection.h clientConnection.cpp \
	connection.cpp connection.h \
	connection_fwd.h \
	dataMarshaller.h dataMarshaller.cpp \
	displayInfo.cpp displayInfo.h \
	edgeDisplayInfo.cpp edgeDisplayInfo.h \
	labelDisplayInfo.cpp labelDisplayInfo.h \
	floatingLabelDisplayInfo.cpp floatingLabelDisplayInfo.h \
	messageHandler.h messageHandlerFwd.h messageHandler.cpp \
	sendMessageHandler.h \
	messageFactory.h messageFactory.cpp \
	messageStatus.cpp messageStatus.h \
	messageStream.h messageStream.cpp \
	messageStreamFilter.h messageStreamFilter.cpp \
	messageStreamReactor.cpp messageStreamReactor.h \
	nodeDisplayInfo.cpp nodeDisplayInfo.h \
	watcherGlobalFunctions.cpp watcherGlobalFunctions.h \
	watcherGraph.cpp watcherGraph.h \
	watcherLayerData.cpp watcherLayerData.h \
	watcherRegion.h watcherRegion.cpp \
	watcherdAPIMessageHandler.h watcherdAPIMessageHandler.cpp \
	watcherTypes.cpp watcherTypes.h \
	watcherColors.cpp watcherColors.h \
	colors.cpp colors.h \
	marshalYAML.h marshalYAML.cpp 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = watcher.pc

