# GTL - experimental automatic use of multiprocessor systems to speed up make 
# May break things, don't know. 
#PROCCOUNT=$(shell cat /proc/cpuinfo | grep -c ^processor) 
#ifneq (${PROCCOUNT}, 0) 
#	MAKEFLAGS+=-j $(PROCCOUNT) 
#endif 
SUBDIRS=. test

# "Fixes" warning in log4cxx about type punning. 
CXXFLAGS += -fno-strict-aliasing

CPPFLAGS += -I../../include
CPPFLAGS += @BOOST_CPPFLAGS@

noinst_LIBRARIES=libwatcher.a

libwatcher_a_SOURCES=seekWatcherMessage.cpp speedWatcherMessage.cpp startWatcherMessage.cpp \
	stopWatcherMessage.cpp nodeStatusMessage.cpp \
	edgeMessage.cpp labelMessage.cpp watcherColors.cpp colorMessage.cpp \
	gpsMessage.cpp messageStatus.cpp dataRequestMessage.cpp message.cpp \
	messageTypesAndVersions.cpp testMessage.cpp \
	watcherRegion.cpp 
