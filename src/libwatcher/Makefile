# GTL - experimental automatic use of multiprocessor systems to speed up make 
# May break things, don't know. 
PROCCOUNT=$(shell cat /proc/cpuinfo | grep -c ^processor) 
ifneq (${PROCCOUNT}, 0) 
	MAKEFLAGS+=-j $(PROCCOUNT) 
endif 

CPPFLAGS += -Wall -g -O0 -ggdb -Weffc++
CPPFLAGS += -I../../include

# watcherd API
SRCS=seekWatcherMessage.cpp speedWatcherMessage.cpp startWatcherMessage.cpp \
	stopWatcherMessage.cpp nodeStatusMessage.cpp

# feeder API
SRCS += edgeMessage.cpp labelMessage.cpp watcherColors.cpp colorMessage.cpp \
	gpsMessage.cpp messageStatus.cpp dataRequestMessage.cpp message.cpp \
	messageTypesAndVersions.cpp testMessage.cpp

OBJS=${SRCS:.cpp=.o}

.PHONY : all clean tags

all:: .deps

all:: libwatcher.a

libwatcher.a: ${OBJS}
	ar cr $@ ${OBJS}

clean:
	$(RM) -f libwatcher.a ${OBJS} .deps/*

tags: 
	ctags -R ..

# no errors if .deps/*.d doesn't exist
-include .deps/*.d

.deps:
	@mkdir .deps

.cpp.o:
	${CXX} -c ${CPPFLAGS} -MMD -MF .deps/$*.d -o $@ $<
