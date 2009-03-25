# melkins - compilation for Fedora 10
F10=0

TOPDIR=../..

ifneq (${F10}, 1)
LIBS+=-L/usr/local/lib 
endif
LIBS+=-L$(TOPDIR)/lib
LIBS+=-L../libwatcher

ifneq (${F10}, 1)
CXXFLAGS+=-I/usr/local/include/boost-1_36
endif
CXXFLAGS+=-I$(TOPDIR)/include 
CXXFLAGS+=-I$(TOPDIR)/src/util
CXXFLAGS+=-I..
CXXFLAGS+=-I.
CXXFLAGS+=-O0 -ggdb
CXXFLAGS+=-Wall
CXXFLAGS+=-pthread

LDFLAGS+=-lwatcher
LDFLAGS+=-lconfig++
LDFLAGS+=-llogger
LDFLAGS+=-llog4cxx
LDFLAGS+=-lwatcherutils
ifeq (${F10}, 1)
LDFLAGS+=-lboost_system-mt
LDFLAGS+=-lboost_thread-mt
LDFLAGS+=-lboost_serialization-mt
else
LDFLAGS+=-lboost_system-gcc41-mt
LDFLAGS+=-lboost_thread-gcc41-mt
LDFLAGS+=-lboost_serialization-gcc41-mt
endif

# GTL - experimental automatic use of multiprocessor systems to speed up make
# May break things, don't know.
PROCCOUNT=$(shell cat /proc/cpuinfo | grep -c ^processor)
ifneq (${PROCCOUNT}, 0)
	MAKEFLAGS+=-j $(PROCCOUNT)
endif

#
# SRCS and HDRS come in pairs, MAINS contain main() and do not have an associated header.
#
SRCS=messageFactory.cpp singletonConfig.cpp \
	 serverConnection.cpp dataMarshaller.cpp server.cpp \
	 clientConnection.cpp messageHandler.cpp messageStatusHandler.cpp \
	 gpsMessageHandler.cpp messageHandlerFactory.cpp client.cpp \
	 labelMessageHandler.cpp  edgeMessageHandler.cpp \
	 colorMessageHandler.cpp watcherd.cpp dataRequestMessageHandler.cpp
HDRS=messageFactory.h singletonConfig.h \
	serverConnection.h dataMarshaller.h server.h \
	clientConnection.h messageHandler.h messageStatusHandler.h gpsMessageHandler.h \
	messageHandlerFactory.h client.h \
	labelMessageHandler.h edgeMessageHandler.h \
	colorMessageHandler.h watcherd.h dataRequestMessage.h dataRequestMessageHandler.h
OBJS=$(SRCS:.cpp=.o)

MAINS=gpsMessageTest.cpp testCC.cpp labelMessageTest.cpp edgeMessageTest.cpp colorMessageTest.cpp dataRequestMessageTest.cpp

BINS=$(MAINS:.cpp=) watcherd

all: $(BINS) watcherd.cfg log.properties 

.phony: clean

watcherd: $(OBJS) watcherdMain.o
	$(CXX) $^ -o $@ $(LIBS) $(LDFLAGS)

gpsMessageTest: $(OBJS) gpsMessageTest.o
	$(CXX) $^ -o $@ $(LIBS) $(LDFLAGS)

labelMessageTest: $(OBJS) labelMessageTest.o
	$(CXX) $^ -o $@ $(LIBS) $(LDFLAGS)

edgeMessageTest: $(OBJS) edgeMessageTest.o
	$(CXX) $^ -o $@ $(LIBS) $(LDFLAGS)

colorMessageTest: $(OBJS) colorMessageTest.o
	$(CXX) $^ -o $@ $(LIBS) $(LDFLAGS)

dataRequestMessageTest: $(OBJS) dataRequestMessageTest.o
	$(CXX) $^ -o $@ $(LIBS) $(LDFLAGS)

testCC: $(OBJS) testCC.o
	$(CXX) $^ -o testCC $(LIBS) $(LDFLAGS)

watcherd.cfg: 
	@ echo Creating watcherd.cfg from sample configuration at ../../etc/watcherd.cfg.sample
	cp ../../etc/watcherd.cfg.sample ./watcherd.cfg

log.properties: 
	@ echo Creating log.properties from sample configuration in ../../etc/watcher.log.properties.sample
	cp ../../etc/watcher.log.properties.sample ./log.properties

clean: 
	$(RM) $(BINS) $(OBJS) $(MAINS:.cpp=.o) watcherdMain.o 

tags: $(SRCS) $(HDRS)
	ctags -R . ../../include

DEPENDSOURCES:=${shell find . -name \*.cpp -o -name \*.c -o -name \*.h -o -name \*.hpp}
depend: 
	@echo Building dependencies...
	@$(RM) depend
	@$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $(CFLAGS) ${DEPENDSOURCES} 2>/dev/null > $@;
	@for f in ${OBJS} ; do echo "$$f: Makefile" >> $@; done

-include depend

