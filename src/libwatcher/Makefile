CPPFLAGS += -Wall -g -O0 -ggdb -Weffc++
CPPFLAGS += -I../../include

# watcherd API
SRCS=seekWatcherMessage.cpp speedWatcherMessage.cpp startWatcherMessage.cpp \
	stopWatcherMessage.cpp

# feeder API
SRCS += edgeMessage.cpp labelMessage.cpp watcherColors.cpp colorMessage.cpp \
	gpsMessage.cpp messageStatus.cpp dataRequestMessage.cpp

OBJS=${SRCS:.cpp=.o}

libwatcher.a: ${OBJS}
	ar cr $@ ${OBJS}

clean:
	rm -f libwatcher.a ${OBJS}
