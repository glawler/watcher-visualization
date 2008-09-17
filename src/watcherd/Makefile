
TOPDIR=../..

LIBS+=-L/usr/local/lib 
LIBS+=-L$(TOPDIR)/lib

CXXFLAGS+=-I/usr/local/include/boost-1_36
CXXFLAGS+=-I$(TOPDIR)/include 
CXXFLAGS+=-I$(TOPDIR)/src/util
CXXFLAGS+=-I.
CXXFLAGS+=-O0 -ggdb
CXXFLAGS+=-Wall

LDFLAGS+=-lconfig++
LDFLAGS+=-llogger
LDFLAGS+=-llog4cxx
LDFLAGS+=-lwatcherutils
LDFLAGS+=-lboost_system-gcc41-mt
LDFLAGS+=-lboost_thread-gcc41-mt
LDFLAGS+=-lboost_serialization-gcc41-mt
LDFLAGS+=-lpthread

SRCS=message.cpp messageFactory.cpp messageTypesAndVersions.cpp singletonConfig.cpp testMessage.cpp \
	 serverConnection.cpp dataMarshaller.cpp watcherd.cpp server.cpp \
	 messageStatus.cpp clientConnection.cpp gpsMessage.cpp messageHandler.cpp messageStatusHandler.cpp \
	 gpsMessageHandler.cpp testMessageHandler.cpp messageHandlerFactory.cpp
HDRS=messageFactory.h message.h messageTypesAndVersions.h singletonConfig.h testMessage.h \
	serverConnection.hpp dataMarshaller.hpp server.hpp messageStatus.h \
	clientConnection.hpp gpsMessage.h messageHandler.h messageStatusHandler.h gpsMessageHandler.h \
	testMessageHandler.h messageHandlerFactory.h
OBJS=$(SRCS:.cpp=.o)

BINS=watcherd testClientConnection

all: depend tags $(BINS) watcherd.cfg log.properties 

watcherd: $(OBJS) depend
	$(CXX) $(OBJS) -o watcherd $(LIBS) $(LDFLAGS)

testClientConnection: message.o messageTypesAndVersions.o testMessage.o dataMarshaller.o messageStatus.o clientConnection.o ./test/testCC.o gpsMessage.o
	$(CXX) message.o messageTypesAndVersions.o testMessage.o dataMarshaller.o messageStatus.o clientConnection.o ./gpsMessage.o ./test/testCC.o -o testClientConnection $(LIBS) $(LDFLAGS)

watcherd.cfg: 
	@ echo Creating watcherd.cfg from sample configuration at ../../etc/watcherd.cfg.sample
	cp ../../etc/watcherd.cfg.sample ./watcherd.cfg

log.properties: 
	@ echo Creating log.properties from sample configuration in ../../etc/watcher.log.properties.sample
	cp ../../etc/watcher.log.properties.sample ./log.properties

depend: $(SRCS)
	@echo makedepend
	@$(RM) depend
	@makedepend -f- ${HDRS} ${SRCS} > $@ 2>/dev/null;
	@for f in ${OBJS} depend ; do echo "$$f: Makefile" >> $@; done

clean: 
	$(RM) $(BINS) depend $(OBJS)

tags: $(SRCS) $(HDRS)
	ctags -R . ../../include

# Use the shell to set DEPEND so we don't get spurrious warnings when
# the "depend" file does not exist
#
DEPEND=$(shell if [ -r depend ] ; then echo -n 1; else echo -n 0; fi;)
ifeq (${DEPEND}, 1)
	include depend
endif

