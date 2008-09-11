
BVER=gcc41-mt
LDFLAGS+=-L../../../lib -L/usr/local/lib
LDFLAGS+=-llog4cxx -lpthread -llogger
LDFLAGS+=-lboost_serialization-gcc41-mt
#LDFLAGS+=-lboost_system-gcc41-mt
#LDFLAGS+=-lboost_thread-gcc41-mt
CXXFLAGS+=-I.. -I/usr/local/include/boost-1_36 -I../../../include 
CXXFLAGS+=-O0 -ggdb

testMessageSrc=../messageTypesAndVersions.cpp ../testMessage.cpp ../message.cpp ../messageFactory.cpp
testMessageInc=$(testMessageSrc:.cpp=.h)
testMessageObjs=$(testMessageSrc:.cpp=.o) testTestMessage.o

testTestMessage: 
	$(CXX) $(CXXFLAGS) $(testMessageSrc) testTestMessage.cpp -o testMessage $(LIBS) $(LDFLAGS)

clean: 
	rm -f $(testMessageObjs) testMessage
