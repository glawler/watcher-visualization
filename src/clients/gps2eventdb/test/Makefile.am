include $(top_srcdir)/Makefile.top

LDADD = $(top_srcdir)/libwatcher/libwatchermsg.la
LDADD += $(top_srcdir)/libwatcher/libwatcher.a
LDADD += $(top_srcdir)/util/libwatcherutils.a
LDADD += @LOGGER_LIBS@

BUILT_SOURCES=\
	test.log.properties 

check_PROGRAMS=\
	testFileParser

TESTS=$(check_PROGRAMS)

testFileParser_SOURCES=testFileParser.cpp
testFileParser_LDADD=../fileparser.o

