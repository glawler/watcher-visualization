
SUBDIRS=. test

# "Fixes" warning in log4cxx about type punning. 
CXXFLAGS += -fno-strict-aliasing

CPPFLAGS += -I../../include 		# lob4cxx, libconfig++
CPPFLAGS += -I../util				# watcherutils
CPPFLAGS += -I../logger				# logger
CPPFLAGS += -I../watcherd			# messageFilter, etc. 
CPPFLAGS += -I..
CPPFLAGS += @BOOST_CPPFLAGS@

noinst_LIBRARIES=libwatcher.a

libwatcher_a_SOURCES= \
	colorMessage.cpp \
	connectivityMessage.cpp \
	dataRequestMessage.cpp \
	displayInfo.cpp \
	edgeDisplayInfo.cpp \
	edgeMessage.cpp \
	gpsMessage.cpp \
	labelDisplayInfo.cpp \
	labelMessage.cpp \
	message.cpp \
	messageStatus.cpp \
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
	watcherGraphNode.cpp 

