# "Fixes" warning in log4cxx about type punning. 
CPPFLAGS += -fno-strict-aliasing
CXXFLAGS += -I../../include

lib_LIBRARIES=liblogger.a

liblogger_a_SOURCES=logger.cpp logger.h
liblogger_a_includedir=$(includedir)
liblogger_a_include_HEADERS=logger.h

