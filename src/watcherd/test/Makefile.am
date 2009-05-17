TOPDIR=../../..

CPPFLAGS += -I$(TOPDIR)/src/watcherd
CPPFLAGS += -I$(TOPDIR)/src
CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += -I$(TOPDIR)/src/logger
CPPFLAGS += $(BOOST_CPPFLAGS)

DEFS += -DBOOST_TEST_DYN_LINK

LDFLAGS += $(BOOST_LDFLAGS)
LDFLAGS += -L$(TOPDIR)/src/watcherd
LDFLAGS += -L$(TOPDIR)/src/libwatcher
LDFLAGS += -L$(TOPDIR)/lib
LDFLAGS += -L$(TOPDIR)/src/logger
LDADD = -lwatcherdAPI -lwatcher -llogger -llog4cxx
LDADD += $(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_SYSTEM_LIB)

TESTS=testMessageStream testWatcherGraph
bin_PROGRAMS=testMessageStream testWatcherGraph
testMessageStream_SOURCES=testMessageStream.cpp
testWatcherGraph_SOURCES=testWatcherGraph.cpp

