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


all: libwatcher.a

.PHONY : clean tags

libwatcher.a: ${OBJS}
	ar cr $@ ${OBJS}

clean:
	$(RM) -f libwatcher.a ${OBJS} 

tags: 
	ctags -R ..

DEPENDSOURCES:=${shell find . -name \*.cpp -o -name \*.c -o -name \*.h -o -name \*.hpp}
depend: 
	@echo Building dependencies...
	@$(RM) depend
	@$(CXX) -MM $(CPPFLAGS) $(CFLAGS) ${DEPENDSOURCES} 2>/dev/null > $@; 
	@for f in ${OBJS}  ; do echo "$$f: Makefile" >> $@; done

# no errors if "depend" doesn't exist
-include depend

