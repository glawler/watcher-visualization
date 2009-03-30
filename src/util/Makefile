CXXFLAGS+=-I../../include -O0 -ggdb

LIB=../../lib/libwatcherutils.a

SRCS=$(shell echo *.cpp)
HDRS=$(shell echo *.h)
OBJS=$(SRCS:.cpp=.o)

all: $(LIB)

$(LIB): $(OBJS) $(HDRS)
	$(AR) rv $@ $^
	ranlib $@

clean: 
	$(RM) $(OBJS) $(LIB)


