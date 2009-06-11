include $(top_srcdir)/Makefile.top

CXXFLAGS += -Wno-deprecated

DEFS += -DBOOST_TEST_DYN_LINK

LDADD = $(top_srcdir)/libwatcher/libwatcher.a
LDADD += $(top_srcdir)/libwatcher/libwatchermsg.la
LDADD += $(top_srcdir)/util/libwatcherutils.a
LDADD += $(top_srcdir)/logger/liblogger.a

LIBS += $(BOOST_UNIT_TEST_FRAMEWORK_LIB)

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
