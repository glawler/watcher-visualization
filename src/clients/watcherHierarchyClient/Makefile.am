
include $(srcdir)/../Makefile.clients

CXXFLAGS += @BOOST_CPPFLAGS@
LDFLAGS += @BOOST_LDFLAGS@
LIBS+=$(BOOST_ASIO_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)
LIBS+=$(LIBYAML_LIB)

BUILT_SOURCES=watcherHierarchyClient.log.properties

# We have to get our build/include stuff sorted out.
CXXFLAGS+=-I../../include 
CXXFLAGS+=-I../../libwatcher 
CXXFLAGS+=-I../../watcherd
CXXFLAGS+=@LOGGER_CFLAGS@
CXXFLAGS+=@idsCommunications_CFLAGS@
CXXFLAGS+=-I../..

CFLAGS+=$(CXXFLAGS)

LDFLAGS+=-L../../libwatcher
LDFLAGS+=-L../../watcherd
LDFLAGS+=@LOGGER_LIBS@

LIBS+=-lwatcher
LIBS+=-lwatchermsg
LIBS+=-lwatcher
LIBS+=@idsCommunications_LIBS@
LIBS+=$(BOOST_PROGRAM_OPTIONS_LIB)

bin_PROGRAMS=watcherHierarchyClient

watcherHierarchyClient_SOURCES=\
	demolib.h \
	demolib.c \
	legacyWatcherMessageUnmarshal.cpp \
	legacyWatcherMessageUnmarshal.h \
	marshal.h \
	watcherHierarchyClient.cpp \
	configuration.cpp configuration.h \
	AlertHandler.cpp AlertHandler.h  \
	AlertHandlerDefault.cpp AlertHandlerDefault.h  \
	AlertHandlerFictitiousNeighbor.cpp AlertHandlerFictitiousNeighbor.h  \
	AlertHandlerOmittedNeighbor.cpp AlertHandlerOmittedNeighbor.h  \
	AlertHandlers.cpp AlertHandlers.h  \
	AlertHandlerWormhole.cpp AlertHandlerWormhole.h \
	XPathExtractor.cpp XPathExtractor.h \
	alertClassifications.h

