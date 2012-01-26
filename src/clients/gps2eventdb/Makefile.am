include $(srcdir)/../Makefile.clients

LIBS+=$(BOOST_SYSTEM_LIB) $(BOOST_PROGRAM_OPTIONS_LIB)

BUILT_SOURCES=

bin_PROGRAMS=gps2eventdb

gps2eventdb_SOURCES=\
	main.cpp \
	fileparser.h fileparser.cpp \
	configuration.h configuration.cpp \
	CEarth.hpp CEarth.cpp \
	CEarthCoordinate.cpp GFC.h \
	CPolarCoordinate.cpp \
	CMath.inl CMath.hpp 

gps2eventdb_LDADD=\
	@SQLITE3_LIBS@ \
	@LIBYAML_LIBS@ \
	../../watcherd/database.o \
	../../watcherd/sqliteDatabase.o \
	../../watcherd/watcherdConfig.o \
	../../sqlite_wrapper/libsqlite_wrapper.a \
	../../libwatcher/libwatcher.a \
	../../util/libwatcherutils.a 


gps2eventdb_CPPFLAGS=\
	@SQLITE3_CFLAGS@ \
	@LIBYAML_CFLAGS@ \
	-I../../watcherd \
	-I../../sqlite_wrapper \
	-I../../libwatcher

