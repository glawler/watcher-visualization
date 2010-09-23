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

bin_SCRIPTS=watcherClientLoadAverage.py watcherClientBandwidth.py

# How do we tell the make system that the default foo_PROGRAM file is foo.cpp instead of foo.c? 
sendGPSMessage_SOURCES=sendGPSMessage.cpp
sendLabelMessage_SOURCES=sendLabelMessage.cpp
sendEdgeMessage_SOURCES=sendEdgeMessage.cpp
sendColorMessage_SOURCES=sendColorMessage.cpp
sendConnectivityMessage_SOURCES=sendConnectivityMessage.cpp
sendDataPointMessage_SOURCES=sendDataPointMessage.cpp
sendNodePropertiesMessage_SOURCES=sendNodePropertiesMessage.cpp
showClock_SOURCES=showClock.cpp

# GTL - pingWatcher is fundamentally flawed. Just looking 
# at pings you cannot determine anything other than
# src and dst - no intermediate hops. Possibly be able to 
# parse the internals of a ping -R, but, that seems wrong. 
# Maybe traceroute IMCP packets would be a better candidate.
# In the meantime, don't build pingWatcher.
#pingWatcher_SOURCES=pingWatcher.cpp
#pingWatcher_LDFLAGS=-lpcap


