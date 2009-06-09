# melkins - compilation for Fedora 10
-include ../Makefile.local

TOPDIR=../..
SRCDIR=$(TOPDIR)/src

# "Fixes" warning in log4cxx about type punning. 
CPPFLAGS += -fno-strict-aliasing

CPPFLAGS += -I..
CPPFLAGS += -I../util
CPPFLAGS += -I../logger
CPPFLAGS += -I../sqlite_wrapper
CPPFLAGS += -I$(TOPDIR)/include
CPPFLAGS += $(BOOST_CPPFLAGS)

LDFLAGS += -L$(TOPDIR)/lib
LDFLAGS += $(BOOST_LDFLAGS)

# system libraries
LIBS+=-lconfig++
LIBS+=-llog4cxx
LIBS+=-lsqlite3
LIBS+=$(BOOST_ASIO_LIB) $(BOOST_SERIALIZATION_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_FILESYSTEM_LIB)

# 
# .cfg and .log.properties build rules are below
#
BUILT_SOURCES= \
	watcherd.cfg \
	watcherd.log.properties 

bin_PROGRAMS=watcherd 

watcherd_SOURCES=\
	watcherdMain.cpp \
	watcherd.cpp \
	server.cpp \
	serverConnection.cpp \
	serverMessageHandler.cpp \
	writeDBMessageHandler.cpp \
	writeDBMessageHandler.h \
	database.h \
	database.cpp \
	sqliteDatabase.h \
	sqliteDatabase.cpp \
	watcherdConfig.h \
	watcherdConfig.cpp \
	replayState.h \
	replayState.cpp

watcherd_LDADD = ../libwatcher/libwatcher.a 
watcherd_LDADD += ../libwatcher/libwatchermsg.la 
watcherd_LDADD += ../sqlite_wrapper/libsqlite_wrapper.a
watcherd_LDADD += ../util/libwatcherutils.a 
watcherd_LDADD += ../logger/liblogger.a

ctags:
	ctags -R $(TOPDIR)

#
# Copy sample cfg and log.properties files from $(TOPDIR)/etc
#
%.cfg : 
	@ echo Creating $*.cfg from sample configuration at $(TOPDIR)/etc/$*.cfg.sample
	cp -v $(TOPDIR)/etc/$*.cfg.sample ./$*.cfg

%.log.properties: 
	@ echo Creating $*.log.properties from sample configuration at ../etc/watcher.log.properties.sample
	cp -v $(TOPDIR)/etc/watcherd.log.properties.sample ./$*.log.properties
	@ echo This is probably a bad idea - attempting to set the correct output log file in log.properties.
	@ perl -pi -e 's/watcherd.log/$*.log/g' ./$*.log.properties
