CXXFLAGS+=-I../../include 
LDFLAGS+=-L../../lib -llog4cxx

TARGET=../../lib/liblogger.a

all: $(TARGET)

../../lib/liblogger.a: logger.o
	$(AR) rv $@ $^
	ranlib $@

logger.o: logger.cpp ../../include/logger.h

clean: 
	$(RM) logger.o ../../lib/liblogger.a


