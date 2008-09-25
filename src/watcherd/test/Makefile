
BVER=gcc41-mt
LDFLAGS+=-L../../../lib -L/usr/local/lib
LDFLAGS+=-llog4cxx -lpthread -llogger
LDFLAGS+=-lboost_serialization-gcc41-mt
LDFLAGS+=-lboost_system-gcc41-mt
LDFLAGS+=-lboost_thread-gcc41-mt
LDFLAGS+=-lboost_unit_test_framework-gcc41-mt
CXXFLAGS+=-I.. -I/usr/local/include/boost-1_36 -I../../../include 
CXXFLAGS+=-O0 -ggdb -Wall
CXXFLAGS+=-DBOOST_TEST_DYN_LINK

testMessageSrc=../messageTypesAndVersions.cpp ../testMessage.cpp ../message.cpp ../messageFactory.cpp ../messageStatus.cpp
testMessageInc=$(testMessageSrc:.cpp=.h)
testMessageObjs=$(testMessageSrc:.cpp=.o) testTestMessage.o

testCCSrc=../clientConnection.cpp ../message.cpp ../testMessage.cpp ../dataMarshaller.cpp ../messageTypesAndVersions.cpp
testCCInc=../clientConnection.hpp ../message.hpp ../testMessage.hpp ../dataMarshaller.hpp ../messageTypesAndVersions.h
testCCObjs=$(testCCSrc:.cpp=.o) testCC.o

colorTestSrc=../watcherColors.cpp
colorTestInc=../watcherColors.h 
colorTestObjs=$(colorTestSrc:.cpp=.o) colorTest.o

labelMessageTestSrc=../message.cpp ../labelMessage.cpp ../watcherColors.cpp ../messageTypesAndVersions.cpp ../watcherGlobalFunctions.cpp
labelMessageTestInc=../message.cpp ../labelMessage.cpp ../watcherColors.h ../messageTypesAndVersions.h ../watcherGlobalFunctions.h
labelMessageTestObjs=$(labelMessageTestSrc:.cpp=.o) testLabelMessage.o

edgeMessageTestSrc=../message.cpp ../edgeMessage.cpp ../watcherColors.cpp ../messageTypesAndVersions.cpp ../watcherGlobalFunctions.cpp
edgeMessageTestInc=../message.cpp ../edgeMessage.cpp ../watcherColors.h ../messageTypesAndVersions.h ../watcherGlobalFunctions.h
edgeMessageTestObjs=$(edgeMessageTestSrc:.cpp=.o) testEdgeMessage.o

ALL_OBJS=$(colorTestObjs) $(testCCObjs) $(testMessageObjs) $(labelMessageTestObjs) $(edgeMessageTestObjs)

BINS=colorTest testLabelMessage testEdgeMessage

all: $(BINS)

colorTest: $(colorTestObjs)
	$(CXX) $(CXXFLAGS) $^ -o colorTest $(LIBS) $(LDFLAGS)

testCC: $(testCCObjs) 
	$(CXX) $(CXXFLAGS) $(testCCObjs) -o testClientConnection $(LIBS) $(LDFLAGS)

testLabelMessage: $(labelMessageTestObjs)
	$(CXX) $^ -o testLabelMessage $(LIBS) $(LDFLAGS)

testEdgeMessage: $(edgeMessageTestObjs)
	$(CXX) $^ -o testEdgeMessage $(LIBS) $(LDFLAGS)

testTestMessage: 
	$(CXX) $(CXXFLAGS) $(testMessageSrc) testTestMessage.cpp -o testMessage $(LIBS) $(LDFLAGS)

test: $(BINS)
	@for b in $(BINS); do ./$$b; done; 

clean: 
	rm -f $(BINS) $(ALL_OBJS)

depend: $(SRCS)
	@echo makedepend
	@$(RM) depend
	@makedepend -f- *.cpp *.h ../*.cpp ../*.h* > $@ 2>/dev/null;
	@for f in ${OBJS} depend ; do echo "$$f: Makefile" >> $@; done

DEPEND=$(shell if [ -r depend ] ; then echo -n 1; else echo -n 0; fi;)
ifeq (${DEPEND}, 1)
	include depend
endif


