CPPFLAGS += -Wall -g -O0 -ggdb -Weffc++
CPPFLAGS += -I../../include
SRCS=seekWatcherMessage.cpp speedWatcherMessage.cpp startWatcherMessage.cpp \
	stopWatcherMessage.cpp

OBJS=${SRCS:.cpp=.o}

libwatcher.a: ${OBJS}
	ar cr $@ ${OBJS}

clean:
	rm -f libwatcher.a ${OBJS}
