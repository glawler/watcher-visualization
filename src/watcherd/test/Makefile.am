TOPDIR=../../..

CPPFLAGS += -I$(TOPDIR)/src/watcherd
CPPFLAGS += -I$(TOPDIR)/src
CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += -I$(TOPDIR)/src/logger
CPPFLAGS += -I$(TOPDIR)/src/util
CPPFLAGS += $(BOOST_CPPFLAGS)

DEFS += -DBOOST_TEST_DYN_LINK

LDFLAGS += $(BOOST_LDFLAGS)
LDFLAGS += -L$(TOPDIR)/src/logger
LDFLAGS += -L$(TOPDIR)/src/util
LDFLAGS += -L$(TOPDIR)/src/watcherd
LDFLAGS += -L$(TOPDIR)/src/libwatcher
LDFLAGS += -L$(TOPDIR)/lib
LDFLAGS += -L$(TOPDIR)/src/logger
LDADD = -lwatcherdAPI -lwatcher -llogger -llog4cxx -lconfig++ -lwatcherutils
LDADD += $(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB)

# LDADD autodependency not working on Fedora 8
DEPENDENCIES= \
	../libwatcherdAPI.a \
	$(TOPDIR)/src/libwatcher/libwatcher.a

BUILT_SOURCES= \
	test.log.properties

TESTS= \
	testMessageStream \
	testWatcherGraph \
	testWatcherGraphNode \
	testWatcherGraphEdge \
	testLoadSaveDisplayInfo

bin_PROGRAMS= \
	testMessageStream \
	testWatcherGraph \
	testWatcherGraphNode \
	testWatcherGraphEdge \
	testLoadSaveDisplayInfo

testMessageStream_SOURCES=testMessageStream.cpp
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
