-include Makefile.local

BVER=gcc41-mt
LDFLAGS+=-L../../../lib
LDFLAGS+=-llog4cxx -llogger
ifeq ($(F10),1)
LDFLAGS+=-lboost_serialization-mt
LDFLAGS+=-lboost_system-mt
LDFLAGS+=-lboost_thread-mt
LDFLAGS+=-lboost_unit_test_framework-mt
else
LDFLAGS+=-L/usr/local/lib
LDFLAGS+=-lboost_serialization-gcc41-mt
LDFLAGS+=-lboost_system-gcc41-mt
LDFLAGS+=-lboost_thread-gcc41-mt
LDFLAGS+=-lboost_unit_test_framework-gcc41-mt
CXXFLAGS+=-I/usr/local/include/boost-1_36
endif
CXXFLAGS+=-I.. -I../../../include 
CXXFLAGS+=-O0 -ggdb -Wall -Weffc++
CXXFLAGS+=-DBOOST_TEST_DYN_LINK
CXXFLAGS+=-pthread

WATCHERD_DIR=../../watcherd/

LIBS += -L.. -lwatcher

# GTL - experimental automatic use of multiprocessor systems to speed up make
# May break things, don't know.
#PROCCOUNT=$(shell cat /proc/cpuinfo | grep -c ^processor)
#ifneq (${PROCCOUNT}, 0)
#	MAKEFLAGS+=-j $(PROCCOUNT)
#endif

BINS=testLabelMessage

.PHONY : all clean test

all: .deps $(BINS)

testLabelMessage: testLabelMessage.o
	$(CXX) -o testLabelMessage $^ $(LIBS) $(LDFLAGS)

test: $(BINS)
	@for b in $(BINS); do ./$$b; done; 

clean: 
	rm -f $(BINS) *.o

.deps:
	@mkdir .deps

-include .deps/*.d

.cpp.o:
	${CXX} -c ${CXXFLAGS} -MMD -MF .deps/$*.d -o $@ $<
