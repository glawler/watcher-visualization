TOPDIR=../../..
SRCDIR=$(TOPDIR)/src

CPPFLAGS += -Wno-deprecated

CPPFLAGS += -I../..
CPPFLAGS += -I../../../include
CPPFLAGS += -I../../logger
CPPFLAGS += -I../../util
CPPFLAGS += $(BOOST_CPPFLAGS)

DEFS += -DBOOST_TEST_DYN_LINK

LDFLAGS += -L../../../lib
LDFLAGS += $(BOOST_LDFLAGS)
LDFLAGS += -llog4cxx
LDFLAGS += -lconfig++

# apparently order is important for g++, libwatcherd has things that libwatcher needs. 
LDADD = $(SRCDIR)/libwatcher/libwatcher.a
LDADD += $(SRCDIR)/util/libwatcherutils.a
LDADD += $(SRCDIR)/logger/liblogger.a
LDADD += \
	$(BOOST_ASIO_LIB) \
	$(BOOST_SERIALIZATION_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_UNIT_TEST_FRAMEWORK_LIB) \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_REGEX_LIB)

BUILT_SOURCES=\
	test.log.properties 

bin_PROGRAMS=testCC

check_PROGRAMS=\
	testStartMessage \
	testStopMessage \
	testSeekMessage \
	testSpeedMessage \
	testLabelMessage \
	testEdgeMessage \
	testTestMessage \
	colorTest \
	testConnectivityMessage \
	testWatcherGraph \
	testWatcherGraphNode \
	testWatcherGraphEdge \
	testLoadSaveDisplayInfo \
	testMessageStream

TESTS=$(check_PROGRAMS)

# Is there a way to tell autotools that the default map is progname --> progname.cpp? 
testLabelMessage_SOURCES=testLabelMessage.cpp
testEdgeMessage_SOURCES=testEdgeMessage.cpp
testStartMessage_SOURCES=testStartMessage.cpp
testStopMessage_SOURCES=testStopMessage.cpp
testSeekMessage_SOURCES=testSeekMessage.cpp
testSpeedMessage_SOURCES=testSpeedMessage.cpp
testTestMessage_SOURCES=testTestMessage.cpp
colorTest_SOURCES=colorTest.cpp
testConnectivityMessage_SOURCES=testConnectivityMessage.cpp
testWatcherGraph_SOURCES=testWatcherGraph.cpp
testWatcherGraphNode_SOURCES=testWatcherGraphNode.cpp
testWatcherGraphEdge_SOURCES=testWatcherGraphEdge.cpp
testLoadSaveDisplayInfo_SOURCES=testLoadSaveDisplayInfo.cpp

#
# Copy sample cfg and log.properties files from $(TOPDIR)/etc
#
%.cfg : 
	@ echo Creating $*.cfg from sample configuration at $(TOPDIR)/etc/$*.cfg.sample
	cp -v $(TOPDIR)/etc/$*.cfg.sample ./$*.cfg

%.log.properties: 
	@ echo Creating $*.log.properties from sample configuration at ../etc/watcher.log.properties.sample
	cp -v $(TOPDIR)/etc/watcherd.log.properties.sample ./$*.log.properties
	@ echo This is probably a bad idea - attempting to set the correct output log file in log.properties.
	@ perl -pi -e 's/watcherd.log/$*.log/g' ./$*.log.properties
