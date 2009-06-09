
CPPFLAGS+=-I../.. 				# libwatcher
CPPFLAGS+=-I../../../include 	# log4cxx, etc.
CPPFLAGS+=-I../../util
CPPFLAGS+=-I../../logger
CPPFLAGS+=-I../include
CPPFLAGS+=$(BOOST_CPPFLAGS)

# watcher libs
LDADD =  \
	../../libwatcher/libwatcher.a \
	../../util/libwatcherutils.a \
	../../logger/liblogger.a 
	

# system libraries
LIBS+=-L../../../lib 		# GTL - I have log4cxx and libconfig++ built locally. 
LIBS+=-lconfig++
LIBS+=-llog4cxx
LIBS+=$(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB)

BUILT_SOURCES=sendMessage.log.properties

bin_PROGRAMS= \
	sendGPSMessage \
	sendLabelMessage \
	sendEdgeMessage \
	sendColorMessage \
	sendDataRequestMessage \
	sendConnectivityMessage 

# How do we tell the make system that the default foo_PROGRAM file is foo.cpp instead of foo.c? 
sendGPSMessage_SOURCES=sendGPSMessage.cpp
sendLabelMessage_SOURCES=sendLabelMessage.cpp
sendEdgeMessage_SOURCES=sendEdgeMessage.cpp
sendColorMessage_SOURCES=sendColorMessage.cpp
sendDataRequestMessage_SOURCES=sendDataRequestMessage.cpp
sendConnectivityMessage_SOURCES=sendConnectivityMessage.cpp

ctags:
	ctags -R ../..

%.log.properties: 
	@ echo Creating $*.log.properties from sample configuration at ../etc/watcher.log.properties.sample
	cp -v ../../../etc/watcherd.log.properties.sample ./$*.log.properties
	@ echo This is probably a bad idea - attempting to set the correct output log file in log.properties.
	@ perl -pi -e 's/watcherd.log/$*.log/g' ./$*.log.properties


