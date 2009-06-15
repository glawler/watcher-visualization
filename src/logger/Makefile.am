# "Fixes" warning in log4cxx about type punning. 
CPPFLAGS += -fno-strict-aliasing
CXXFLAGS += -I../../include

lib_LTLIBRARIES=liblogger.la

liblogger_la_SOURCES=logger.cpp logger.h
liblogger_la_includedir=$(includedir)
liblogger_la_include_HEADERS=logger.h

