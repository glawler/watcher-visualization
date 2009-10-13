
include $(srcdir)/../Makefile.clients

CXXFLAGS+=-I/usr/include/libxml2 
LDADD+=-lxml2

CPPFLAGS += @BOOST_CPPFLAGS@
LDFLAGS += @BOOST_LDFLAGS@
LIBS+=$(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

BUILT_SOURCES=watcherHierarchyClient.log.properties

# We have to get our build/include stuff sorted out.
CXXFLAGS+=-I../../include 
CXXFLAGS+=-I../../libwatcher 
CXXFLAGS+=-I../../watcherd
CXXFLAGS+=@LOGGER_CFLAGS@
CXXFLAGS+=-I../../idsCommunications
CXXFLAGS+=-I../..

CFLAGS+=-I../../../include
CFLAGS+=-I/usr/include/libxml2

LDFLAGS+=-L/usr/local/lib/
LDFLAGS+=-L../../libwatcher
LDFLAGS+=-L../../watcherd
LDFLAGS+=-L../../../lib
LDFLAGS+=-L../../idsCommunications
LDFLAGS+=@LOGGER_LIBS@

LIBS+=-lidsCommunications
LIBS+=-lwatcher
LIBS+=-lwatchermsg
LIBS+=-lwatcher
LIBS+=-lidsCommunications

bin_PROGRAMS=watcherHierarchyClient

watcherHierarchyClient_SOURCES=\
	demolib.h \
	demolib.c \
	legacyWatcherMessageUnmarshal.cpp \
	legacyWatcherMessageUnmarshal.h \
	marshal.h \
	watcherGPS.cpp \
	watcherGPS.h \
	watcherHierarchyClient.cpp

