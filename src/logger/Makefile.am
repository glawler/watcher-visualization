# "Fixes" warning in log4cxx about type punning. 
CPPFLAGS += -fno-strict-aliasing
CXXFLAGS += -I../../include -I../../include/apr-1

lib_LTLIBRARIES=liblogger.la

liblogger_la_SOURCES=logger.cpp declareLogger.h logger.h
liblogger_la_includedir=$(includedir)
# only install the fwd decl version of the logger header so as to not pollute/clash with 3rd-party loggers/modules.
liblogger_la_include_HEADERS=declareLogger.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = logger.pc

