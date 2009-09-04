include $(srcdir)/../Makefile.clients

BUILT_SOURCES=sendMessage.log.properties

bin_PROGRAMS= \
	sendGPSMessage \
	sendLabelMessage \
	sendEdgeMessage \
	sendColorMessage \
	sendConnectivityMessage \
	sendDataPointMessage \
	sendNodePropertiesMessage \
	showClock

# How do we tell the make system that the default foo_PROGRAM file is foo.cpp instead of foo.c? 
sendGPSMessage_SOURCES=sendGPSMessage.cpp
sendLabelMessage_SOURCES=sendLabelMessage.cpp
sendEdgeMessage_SOURCES=sendEdgeMessage.cpp
sendColorMessage_SOURCES=sendColorMessage.cpp
sendConnectivityMessage_SOURCES=sendConnectivityMessage.cpp
sendDataPointMessage_SOURCES=sendDataPointMessage.cpp
sendNodePropertiesMessage_SOURCES=sendNodePropertiesMessage.cpp
showClock_SOURCES=showClock.cpp


