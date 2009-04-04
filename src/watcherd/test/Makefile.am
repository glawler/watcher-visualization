TOPDIR=../../..

CPPFLAGS += -I$(TOPDIR)/src/watcherd
CPPFLAGS += -I$(TOPDIR)/src
CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += $(BOOST_CPPFLAGS)

DEFS += -DBOOST_TEST_DYN_LINK

LDFLAGS += $(BOOST_LDFLAGS)
LDFLAGS += -L$(TOPDIR)/src/watcherd
LDFLAGS += -L$(TOPDIR)/src/libwatcher
LDFLAGS += -L$(TOPDIR)/lib
LDFLAGS += -L../../logger
LDADD = -lwatcherdAPI -lwatcher -llogger -llog4cxx
LDADD += $(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_SYSTEM_LIB)

# GTL - test does not currently build, so comment out so I can commit 
TESTS=testMessageStream
bin_PROGRAMS=testMessageStream
testMessageStream_SOURCES=testMessageStream.cpp
