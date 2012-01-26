include $(top_srcdir)/Makefile.top

CXXFLAGS += -Wno-deprecated -std=c++0x
CPPFLAGS += @LIBYAML_CFLAGS@ -I..

DEFS += -DBOOST_TEST_DYN_LINK

LDADD = $(top_srcdir)/libwatcher/libwatcher.a
LDADD += $(top_srcdir)/util/libwatcherutils.a
LDADD += @LOGGER_LIBS@ @LIBYAML_LIBS@

LIBS += $(BOOST_UNIT_TEST_FRAMEWORK_LIB)

BUILT_SOURCES=\
	test.log.properties 

# bin_PROGRAMS=testCC

check_PROGRAMS=\
	testStartMessage \
	testStopMessage \
	testSeekMessage \
	testSpeedMessage \
	testLabelMessage \
	testEdgeMessage \
	testMessage \
	colorTest \
	testConnectivityMessage \
	testLoadSaveDisplayInfo \
	testMessageStream \
	testMessageStreamFilter \
	testYAML \
	testDataMarshal

# GTL - unit tests need to be re-written for watcher graph classes
# testWatcherGraph 
# testWatcherGraphNode 
# testWatcherGraphEdge

TESTS=$(check_PROGRAMS)

# Is there a way to tell autotools that the default map is progname --> progname.cpp? 
testLabelMessage_SOURCES=testLabelMessage.cpp
testEdgeMessage_SOURCES=testEdgeMessage.cpp
testMessage_SOURCES=testMessage.cpp
testStartMessage_SOURCES=testStartMessage.cpp
testStopMessage_SOURCES=testStopMessage.cpp
testSeekMessage_SOURCES=testSeekMessage.cpp
testSpeedMessage_SOURCES=testSpeedMessage.cpp
colorTest_SOURCES=colorTest.cpp
testConnectivityMessage_SOURCES=testConnectivityMessage.cpp
testLoadSaveDisplayInfo_SOURCES=testLoadSaveDisplayInfo.cpp
testMessageStream_SOURCES=testMessageStream.cpp
testMessageStreamFilter_SOURCES=testMessageStreamFilter.cpp
testYAML_SOURCES=testYAML.cpp
testDataMarshal_SOURCES=testDataMarshal.cpp

# GTL - unit tests need to be re-written for watcher graph classes
# testWatcherGraph_SOURCES=testWatcherGraph.cpp
# testWatcherGraphNode_SOURCES=testWatcherGraphNode.cpp
# testWatcherGraphEdge_SOURCES=testWatcherGraphEdge.cpp
