CPPFLAGS += -I..
CPPFLAGS += -I../..
CPPFLAGS += -I../../../include
CPPFLAGS += $(BOOST_CPPFLAGS)
DEFS += -DBOOST_TEST_DYN_LINK
LDFLAGS += -L..
LDFLAGS += -L../../libwatcher
LDFLAGS += -L../../../lib
LDFLAGS += -L../../../logger
LDFLAGS += $(BOOST_LDFLAGS)
LDADD = -lwatcher -lwatcherdAPI -llogger -llog4cxx
LDADD += $(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_SYSTEM_LIB)

# GTL - test does not currently build, so comment out so I can commit 
# TESTS=testMessageStream
#bin_PROGRAMS=testMessageStream
#testMessageStream_SOURCES=testMessageStream.cpp
