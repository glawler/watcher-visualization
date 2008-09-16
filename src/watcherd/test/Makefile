
BVER=gcc41-mt
LDFLAGS+=-L../../../lib -L/usr/local/lib
LDFLAGS+=-llog4cxx -lpthread -llogger
LDFLAGS+=-lboost_serialization-gcc41-mt
LDFLAGS+=-lboost_system-gcc41-mt
LDFLAGS+=-lboost_thread-gcc41-mt
CXXFLAGS+=-I.. -I/usr/local/include/boost-1_36 -I../../../include 
CXXFLAGS+=-O0 -ggdb -Wall

testMessageSrc=../messageTypesAndVersions.cpp ../testMessage.cpp ../message.cpp ../messageFactory.cpp ../messageStatus.cpp
testMessageInc=$(testMessageSrc:.cpp=.h)
testMessageObjs=$(testMessageSrc:.cpp=.o) testTestMessage.o

testCCSrc=../clientConnection.cpp ../message.cpp ../testMessage.cpp ../dataMarshaller.cpp ../messageTypesAndVersions.cpp
testCCInc=../clientConnection.hpp ../message.hpp ../testMessage.hpp ../dataMarshaller.hpp ../messageTypesAndVersions.h
testCCObjs=$(testCCSrc:.cpp=.o) testCC.o

BINS=testTestMessage testCC

all: $(BINS)

testCC: $(testCCObjs) 
	$(CXX) $(CXXFLAGS) $(testCCObjs) -o testClientConnection $(LIBS) $(LDFLAGS)

testTestMessage: 
	$(CXX) $(CXXFLAGS) $(testMessageSrc) testTestMessage.cpp -o testMessage $(LIBS) $(LDFLAGS)

clean: 
	rm -f $(testMessageObjs) $(testCCObjs) testMessage

depend: $(SRCS)
	@echo makedepend
	@$(RM) depend
	@makedepend -f- *.cpp *.h ../*.cpp ../*.h* > $@ 2>/dev/null;
	@for f in ${OBJS} depend ; do echo "$$f: Makefile" >> $@; done

DEPEND=$(shell if [ -r depend ] ; then echo -n 1; else echo -n 0; fi;)
ifeq (${DEPEND}, 1)
	include depend
endif


