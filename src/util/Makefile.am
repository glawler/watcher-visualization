CPPFLAGS += -I../../include
CPPFLAGS += @LOGGER_CFLAGS@
CPPFLAGS += $(BOOST_CPPFLAGS)

lib_LIBRARIES=libwatcherutils.a
libwatcherutils_a_includedir=$(includedir)/libwatcher
libwatcherutils_a_include_HEADERS=\
	initConfig.h singletonConfig.h

libwatcherutils_a_SOURCES= \
	initConfig.cpp initConfig.h \
	singletonConfig.cpp singletonConfig.h
	
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = watcherutils.pc
