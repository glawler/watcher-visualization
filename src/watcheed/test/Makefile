
BVER=gcc41-mt
LDFLAGS+=-L../../../lib -L/usr/local/lib
LDFLAGS+=-llog4cxx -lpthread
CXXFLAGS+=-I.. -I/usr/local/include/boost-1_36 -I../../../include 
CXXFLAGS+=-O0 -ggdb

testTestMessage: ../testMessage.h ../baseMessage.h
	$(CXX) $(CXXFLAGS) testTestMessage.cpp -o testMessage $(LIBS) $(LDFLAGS)

