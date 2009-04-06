CPPFLAGS += -I../..
CPPFLAGS += -I../../../include
CPPFLAGS += $(BOOST_CPPFLAGS)
DEFS += -DBOOST_TEST_DYN_LINK
LDFLAGS += -L..
LDFLAGS += -L../../../lib
LDFLAGS += -L../../logger
LDFLAGS += $(BOOST_LDFLAGS)
LDADD = -lwatcher -llogger -llog4cxx
LDADD += $(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_SYSTEM_LIB)
TESTS=testLabelMessage testEdgeMessage testStartMessage testStopMessage testSeekMessage \
      testSpeedMessage
bin_PROGRAMS=testLabelMessage testEdgeMessage testStartMessage testStopMessage testSeekMessage \
	     testSpeedMessage testTestMessage
testLabelMessage_SOURCES=testLabelMessage.cpp
testEdgeMessage_SOURCES=testEdgeMessage.cpp
testStartMessage_SOURCES=testStartMessage.cpp
testStopMessage_SOURCES=testStopMessage.cpp
testSeekMessage_SOURCES=testSeekMessage.cpp
testSpeedMessage_SOURCES=testSpeedMessage.cpp
testTestMessage_SOURCES=testTestMessage.cpp
