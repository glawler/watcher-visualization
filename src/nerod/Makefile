
LIBS+=-L/usr/local/lib
CXXFLAGS+=-I/usr/local/include/boost-1_36 -I../../include
LDFLAGS+=-L../../lib
LDFLAGS+=-lconfig++
LDFLAGS+=-lboost_system-gcc41-mt -lpthread -lboost_thread-gcc41-mt
SRCS=$(shell echo *.cpp)
HDRS=$(shell echo *.hpp) 
OBJS=$(SRCS:.cpp=.o)
BINS=loggerService

all: $(BINS)

loggerService: $(OBJS)
	$(CXX) $(OBJS) -o loggerService $(LIBS) $(LDFLAGS)

depend: $(SRCS)
	@echo makedepend
	@$(RM) depend
	@makedepend -f- ${HDRS} ${SRCS} > $@ 2>/dev/null;
	@for f in ${OBJS} depend ; do echo "$$f: Makefile" >> $@; done

clean: 
	@$(RM) -f $(BINS) depend $(OBJS)

tags: $(SRCS) $(HDRS)
	ctags $(SRCS) $(HDRS)

# Use the shell to set DEPEND so we don't get spurrious warnings when
# the "depend" file does not exist
#
DEPEND=$(shell if [ -r depend ] ; then echo -n 1; else echo -n 0; fi;)
ifeq (${DEPEND}, 1)
	include depend
endif

