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
LDFLAGS += $(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB)
LDFLAGS += -llog4cxx -lconfig++ 
LDADD = -lwatcherd -lwatcher -lwatcherutils -llogger

BUILT_SOURCES= \
	testCC.cfg \
	testCC.log.properties 
	test.log.properties 

noinst_PROGRAMS=\
	testCC

check_PROGRAMS= \
	testMessageStream

TESTS=\
	testMessageStream

testMessageStream_SOURCES=testMessageStream.cpp
testCC_SOURCES=testCC.cpp

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
