
LIBS+=-L/usr/local/lib 
LIBS+=-L../../lib

CXXFLAGS+=-I/usr/local/include/boost-1_36
CXXFLAGS+=-I../../include 
CXXFLAGS+=-I../util
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

SRCS=message.cpp messageFactory.cpp messageTypesAndVersions.cpp singletonConfig.cpp testMessage.cpp 
HDRS=messageFactory.h message.h messageTypesAndVersions.h singletonConfig.h testMessage.h
HDRS+=connection.hpp
OBJS=$(SRCS:.cpp=.o)
BINS=watcheed server client

all: $(BINS)

watcheed: $(OBJS) watcheed.o
	$(CXX) $(OBJS) watcheed.o -o watcheed $(LIBS) $(LDFLAGS)

server: $(OBJS) server.o
	$(CXX) $(OBJS) server.o -o server $(LIBS) $(LDFLAGS)

client: $(OBJS) client.o
	$(CXX) $(OBJS) client.o -o client $(LIBS) $(LDFLAGS)

depend: $(SRCS)
	@echo makedepend
	@$(RM) depend
	@makedepend -f- watcheed.cpp server.cpp client.cpp ${HDRS} ${SRCS} > $@ 2>/dev/null;
	@for f in ${OBJS} depend ; do echo "$$f: Makefile" >> $@; done

clean: 
	$(RM) $(BINS) depend $(OBJS)

tags: $(SRCS) $(HDRS)
	ctags -R . 

# Use the shell to set DEPEND so we don't get spurrious warnings when
# the "depend" file does not exist
#
DEPEND=$(shell if [ -r depend ] ; then echo -n 1; else echo -n 0; fi;)
ifeq (${DEPEND}, 1)
	include depend
endif

