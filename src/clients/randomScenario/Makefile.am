include $(srcdir)/../Makefile.clients

BUILT_SOURCES=

bin_PROGRAMS=randomScenario

# This doesn;t work, why? GTL
# randomScenario_LIBS=$(BOOST_PROGRAM_OPTIONS_LIB)
LIBS+=$(BOOST_PROGRAM_OPTIONS_LIB)
# randomScenario_LIBS+=@watchermsg_LIBS@ @watcher_LIBS@
# randomScenario_CXXFLAGS=@watchermsg_CFLAGS@ @watcher_CFLAGS@
# randomScenario_LDADD=@watchermsg_LIBS@ @watcher_LIBS@

randomScenario_SOURCES=\
	configuration.cpp configuration.h \
	randomScenario.cpp
