include $(top_srcdir)/Makefile.top

# melkins - compilation for Fedora 10
-include ../Makefile.local

dist_sysconf_DATA=../etc/watcherd.cfg

CPPFLAGS += -I../sqlite_wrapper
CPPFLAGS += @SQLITE3_CFLAGS@ @LIBYAML_CFLAGS@

# system libraries
LIBS += @SQLITE3_LIBS@ @LIBYAML_LIBS@

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
	replayState.h \
	replayState.cpp \
	sqliteDatabase.h \
	sqliteDatabase.cpp \
	watcherdConfig.h \
	watcherdConfig.cpp \
	sharedStream.h \
	sharedStream.cpp \
	sharedStreamFwd.h

watcherd_LDADD = ../libwatcher/libwatcher.a 
watcherd_LDADD += ../sqlite_wrapper/libsqlite_wrapper.a
watcherd_LDADD += ../util/libwatcherutils.a 
