
# "Fixes" warning in log4cxx about type punning. 
#CPPFLAGS += -fno-strict-aliasing

CPPFLAGS += @LOG4CXX_CFLAGS@

lib_LTLIBRARIES=liblogger.la

liblogger_la_includedir=$(includedir)
liblogger_la_include_HEADERS=declareLogger.h logger.h
liblogger_la_SOURCES=logger.cpp declareLogger.h logger.h
liblogger_la_LDFLAGS= -version-info 1:0:0

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = logger.pc

