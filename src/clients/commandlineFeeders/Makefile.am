include $(srcdir)/../Makefile.clients

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
