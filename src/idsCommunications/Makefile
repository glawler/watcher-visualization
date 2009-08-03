#
#  Copyright (C) 2004  Networks Associates Technology, Inc.
#  All rights reserved.
#
# $Id: Makefile,v 1.120 2007/08/17 20:03:12 dkindred Exp $
#

CC=cc
RANLIB ?= ranlib

ALLSRCS:=$(shell ls *{.c,.cpp} | sort)
ALLHEADERS:=$(shell ls *.h | sort)
# of course the .o files come from the source files
ALLOBJS:=$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(ALLSRCS)))

LDSO:=$(shell ls /lib/ld-*.so)

BASEOBJ=des.o metric.o rng.o node.o config.o idmefPrint.o hashtable.o 

MODULEOBJ+=mobility.o
COMPFLAGS+=-DMODULE_MOBILITY

MODULEOBJ+=routing.o
COMPFLAGS+=-DMODULE_ROUTING

MODULEOBJ+=data.o
COMPFLAGS+=-DMODULE_DATA

MODULEOBJ+=flood.o
COMPFLAGS+=-DMODULE_FLOOD

MODULEOBJ+=hello.o
COMPFLAGS+=-DMODULE_HELLO

MODULEOBJ+=packetapi.o
COMPFLAGS+=-DMODULE_PACKETAPI

MODULEOBJ+=watcherGPS.o

#MODULEOBJ+=testtraffic.o
#COMPFLAGS+=-DMODULE_TESTTRAFFIC

LIBOBJS = apisupport.o 
# LIBOBJS += bufferPair.o
LIBOBJS += config.o
LIBOBJS += idsCommunications.o

SIMOBJ=$(BASEOBJ) $(MODULEOBJ) main.o libidsCommunications.a 
LIVEOBJ=$(BASEOBJ) $(MODULEOBJ) livenetwork.o libidsCommunications.a 
AMROUTEOBJ=amroute.o amroutesend.o neighborlist.o

INCLUDES += -I/usr/include/libxml2
INCLUDES += -I/usr/local/include 
INCLUDES += -I/usr/local/include/libxml2
INCLUDES += -I/usr/local/include
INCLUDES += -I/usr/X11R6/include 
COMPFLAGS += $(INCLUDES)
COMPFLAGS += $(shell libnet-config --defines)
COMPFLAGS += -g
COMPFLAGS += -Wall
COMPFLAGS += -Wextra -Wno-unused-parameter
COMPFLAGS += -Wundef
COMPFLAGS += -Wshadow
COMPFLAGS += -Wpointer-arith
COMPFLAGS += -Wcast-qual
COMPFLAGS += -Wsign-compare
# -Wmissing-field-initializers not supported by gcc 3.4.4
#COMPFLAGS += -Wmissing-field-initializers
COMPFLAGS += -Wdisabled-optimization
CFLAGS += -Wbad-function-cast
# -Wc++-compat not supported by gcc 4.0.2
#CFLAGS += -Wc++-compat
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wmissing-declarations

#COMPFLAGS += -O0
COMPFLAGS += -O2
COMPFLAGS += -DUSE_RNG

PACKETPROTECTIONOBJ=
PROTECTIONLIB=
PROTECTION_LDFLAGS=
PROTECTION_INCLUDES=

# testing:
#   first only - ok
#   second only
#   neither
#   both

###################################
# uncomment this block for application level message signing support
# (see signing.txt)
#PROTECTIONLIB=../protection/libnnpprotection.a
#PROTECTION_INCLUDES=-I../protection
#LIVEOBJ += ../protection/libnnpprotection.a
#LIBOBJS += transDuo.o
##LIBOBJS += transformNull.o
#LIBOBJS += transformSign.o
#LIBOBJS += untransformNull.o
#LIBOBJS += untransformSign.o
#BINS+=demosigningaggregator
#BINS+=demosigningdetector
#BINS+=demoverifiedalertreader
###################################

###################################
# uncomment this block for userland packet encryption in the daemon
# LIVEOBJ+=packetProtection.o
# LIVEOBJ+=../protection/libnnpprotection.a
# PROTECTIONLIB=../protection/libnnpprotection.a
# PROTECTION_INCLUDES+=-I../protection
# COMPFLAGS+=-DUSE_PACKETPROTECTION
# COMPFLAGS+=-DNNP
# BINS+=testPacketProtection
# PACKETPROTECTIONOBJ=packetProtection.o
###################################

# GTL - experimental automatic use of multiprocessor systems to speed up make
# May break things, don't know.
PROCCOUNT=$(shell cat /proc/cpuinfo | grep -c ^processor)
ifneq (${PROCCOUNT}, 0)
    MAKEFLAGS+=-j $(PROCCOUNT)
endif

INCLUDES += $(PROTECTION_INCLUDES)

LDFLAGS += -L/usr/local/lib
# the rpath thing breaks executing on the elements.  -TOJ
#  -Wl,-rpath,/usr/local/lib
LDFLAGS += -L/usr/lib/libxml2
# the rpath thing breaks executing on the elements.  -TOJ
#-Wl,-rpath,/usr/lib/libxml2

LDFLAGS += -lidmef
LDFLAGS += -lxml2
LDFLAGS += $(shell libnet-config --libs)
LDFLAGS += -lm
LDFLAGS+=-lpcap

#COMPFLAGS += -pg
#LDFLAGS+= -pg
#
#LDFLAGS+=-L//home/protean/idmanet/lib -lefence

#COMPFLAGS += -DDMALLOC
#LDFLAGS+=-L/usr/local/lib -ldmallocth
#
# Debugging flags:
#
#COMPFLAGS += -DDEBUG_AMROUTE
#COMPFLAGS += -DDEBUG_API
#COMPFLAGS += -DDEBUG_APPROUTING
#COMPFLAGS += -DDEBUG_BFT
#COMPFLAGS += -DDEBUG_CH
#COMPFLAGS += -DDEBUG_DATA
#COMPFLAGS += -DDEBUG_EVENTS
#COMPFLAGS += -DDEBUG_FLOOD
#COMPFLAGS += -DDEBUG_GOODWIN
#COMPFLAGS += -DDEBUG_GRAPHCLUSTER
#COMPFLAGS += -DDEBUG_HELLO
#COMPFLAGS += -DDEBUG_INTERIM
#COMPFLAGS += -DDEBUG_INTERIM2
#COMPFLAGS += -DDEBUG_IO
#COMPFLAGS += -DDEBUG_LINKSTATE
#COMPFLAGS += -DDEBUG_PACKET
#COMPFLAGS += -DDEBUG_PACKETAPI
#COMPFLAGS += -DDEBUG_PROTECTION
#COMPFLAGS += -DDEBUG_ROUTING
#COMPFLAGS += -DDEBUG_SELECT
#COMPFLAGS += -DDEBUG_TESTTRAFFIC
#COMPFLAGS += -DDEBUG_WATCHER
#COMPFLAGS += -DDEBUG_DYNAMICROOTGROUP

%.x : %.c
	splint +posixlib +matchanyintegral +boolint -compdestroy -mustfreeonly -dependenttrans -kepttrans -branchstate -compmempass $(INCLUDES) $<

#GRAPHICSLIBS=-L/usr/X11R6/lib -lGL -lGLU -lglut

# Uncomment the following three lines to enable graphics
#
#COMPFLAGS+=-DGRAPHICS
#BASEOBJ+=graphics.o
#LDFLAGS+=$(GRAPHICSLIBS) 

# # The simulator binaries:
# BINS+=interim2
# BINS+=graphcluster
# BINS+=amroute
# # don't build old interim binaries by default, to avoid accidentally running them
# #BINS+=interim
# BINS+=bft
# BINS+=justneighbors
# # Simulation testing binaries
# BINS+=nopcluster
# BINS+=testhello livetesthello
# 
# # GTL - Don't build gmcluster stuff by default; it requires things outside of this directory. 
# #BINS+=gmcluster livegmcluster
# #BINS+=interimgm liveinterimgm
# #GMCLUSTERLIBS=../minilogger/libminilogger.a ../hierarchyClient/nodeVelocityData/libVelocity.a
# #LDFLAGS+=$(GMCLUSTERLIBS)
# 
# # Live network binaries 
# # don't build old interim binaries by default, to avoid accidentally running them
# #BINS+=liveinterim
# BINS+=liveinterim2
# BINS+=livenopcluster
# # Live network binaries which are not promised to work
# BINS+=liveamroute
# BINS+=livebft
# BINS+=livegraphcluster
# BINS+=livejustneighbors
# 
# Live network example and testing binaries
# BINS+=testapi
# BINS+=demoaggregator 
# BINS+=demodetector 
# BINS+=demorewriter 
# BINS+=demoresponder 
# BINS+=edgetest
# BINS+=floatinglabeltest 
# BINS+=glancer
# BINS+=goodwin 
# BINS+=goodwincreate
# BINS+=goodwinmerge 
# BINS+=goodwintest 
# BINS+=labeltest 
# BINS+=watcherpropertytest
# BINS+=watchergraphtest
# BINS+=pgraphwatcher 
# BINS+=positionweight 
# BINS+=routingdetector
# BINS+=statetest
# BINS+=tealabcontrol
# BINS+=watcher
# BINS+=hierarchy2text

# GTL - does not currently build cleanly on BSD. 
# BINS+=infrasniff


CXXFLAGS += $(COMPFLAGS)
CFLAGS += $(COMPFLAGS)

# GTL - only ever make libidsCommunications.a 
# all: $(BINS)
all: libidsCommunications.a liveinterim

amroute:	$(SIMOBJ) $(AMROUTEOBJ)
		$(CXX) -o amroute $(SIMOBJ) $(AMROUTEOBJ) $(LDFLAGS)

liveamroute: $(LIVEOBJ) $(AMROUTEOBJ)
	$(CXX) -o liveamroute $(AMROUTEOBJ) $(LIVEOBJ) $(LDFLAGS)

interim: $(SIMOBJ) interim.o
	$(CXX) -o interim interim.o $(SIMOBJ) $(LDFLAGS)

interim2: $(SIMOBJ) interim2.o 
	$(CXX) -o interim2 interim2.o $(SIMOBJ) $(LDFLAGS)

justneighbors: $(SIMOBJ) justneighbors.o 
	$(CXX) -o justneighbors justneighbors.o $(SIMOBJ) $(LDFLAGS)

liveinterim: $(LIVEOBJ) interim.o
	$(CXX) -o liveinterim interim.o $(LIVEOBJ) $(LDFLAGS)

liveinterim2: $(LIVEOBJ) interim2.o 
	$(CXX) -o liveinterim2 interim2.o $(LIVEOBJ) $(LDFLAGS)

livejustneighbors: $(LIVEOBJ) justneighbors.o
	$(CXX) -o livejustneighbors justneighbors.o $(LIVEOBJ) $(LDFLAGS)

bft: $(SIMOBJ) bft.o
	$(CXX) -o bft bft.o $(SIMOBJ) $(LDFLAGS)

nopcluster: $(SIMOBJ) nopcluster.o
	$(CXX) -o nopcluster nopcluster.o $(SIMOBJ) $(LDFLAGS)

livenopcluster: $(LIVEOBJ) nopcluster.o
	$(CXX) -o livenopcluster nopcluster.o $(LIVEOBJ) $(LDFLAGS)

testhello: $(SIMOBJ) testhello.o $(SIMOBJ)
	$(CXX) -o testhello testhello.o $(SIMOBJ) $(LDFLAGS)

livetesthello: $(SIMOBJ) testhello.o $(LIVEOBJ)
	$(CXX) -o livetesthello testhello.o $(LIVEOBJ) $(LDFLAGS)

gmcluster.o: gmcluster.cpp gmcluster.h 
	$(CXX) $(CXXFLAGS) $(COMPFLAGS) -I../linkInconsistencyDetector -I../hierarchyClient/nodeVelocityData -c gmcluster.cpp -o gmcluster.o 

gmcluster: $(SIMOBJ) gmcluster.o 
	$(CXX) -o gmcluster gmcluster.o $(SIMOBJ) $(LDFLAGS) 

livegmcluster: $(LIVEOBJ) gmcluster.o 
	$(CXX) -o livegmcluster gmcluster.o $(LIVEOBJ) $(LDFLAGS) 

# gmclustermodule.o and interim2gm.o are just gmcluster.o and interim2.o with MODULE_GCMLUSTER defined. 
# These object files are then combined into the experimental interimgm and liveinterimgm excutables. 

gmclustermodule.o: $(SIMOBJ) gmcluster.cpp gmcluster.h interim2.h
	$(CXX) $(CXXFLAGS) $(COMPFLAGS) -I../linkInconsistencyDetector -I../hierarchyClient/nodeVelocityData -DMODULE_GMCLUSTER -c gmcluster.cpp -o gmclustermodule.o

interim2gm.o: $(SIMOBJ) interim2.cpp interim2.h gmcluster.h
	$(CXX) $(CXXFLAGS) $(COMPFLAGS)  -DMODULE_GMCLUSTER -c interim2.cpp -o interim2gm.o

interimgm: $(SIMOBJ) gmclustermodule.o interim2gm.o
	$(CXX) -o interimgm gmclustermodule.o interim2gm.o $(SIMOBJ) $(LDFLAGS) 

liveinterimgm: $(LIVEOBJ) gmclustermodule.o interim2gm.o
	$(CXX) -o liveinterimgm gmclustermodule.o interim2gm.o $(LIVEOBJ) $(LDFLAGS)


livebft: $(LIVEOBJ) bft.o
	$(CXX) -o livebft bft.o $(LIVEOBJ) $(LDFLAGS)

graphcluster: $(SIMOBJ) graphcluster.o
	$(CXX) -o graphcluster $(SIMOBJ) graphcluster.o $(LDFLAGS)

livegraphcluster: $(LIVEOBJ) graphcluster.o
	$(CXX) -o livegraphcluster $(LIVEOBJ) graphcluster.o $(LDFLAGS)

test: graphics.o test.o
	$(CXX) -o test test.o graphics.o $(LDFLAGS)

watcher: $(BASEOBJ) watcher.o watchermovement.o watcherGraph.o floatinglabel.o libidsCommunications.a $(BASEOBJ) $(MODULEOBJ) 
	$(CXX) -o watcher $(BASEOBJ) $(MODULEOBJ) watcher.o watchermovement.o watcherGraph.o floatinglabel.o -L . -lidsCommunications $(LDFLAGS) $(GRAPHICSLIBS)

hierarchy2text: $(BASEOBJ) hierarchy2text.o libidsCommunications.a 
	$(CXX) -o hierarchy2text hierarchy2text.o -L . -lidsCommunications $(LDFLAGS) $(GRAPHICSLIBS)

tealabcontrol: tealabcontrol.o
	$(CXX) -o tealabcontrol tealabcontrol.o

pgraphwatcher: pgraphwatcher.o libidsCommunications.a
	$(CXX) -o pgraphwatcher pgraphwatcher.o -L . -lidsCommunications  $(LDFLAGS)

testapi: testapi.o libidsCommunications.a demolib.o
	$(CXX) -o testapi testapi.o demolib.o -L . -lidsCommunications $(LDFLAGS)

demodetector: demodetector.o demolib.o libidsCommunications.a
	$(CXX) -o demodetector demodetector.o demolib.o -L . -lidsCommunications  $(LDFLAGS)

goodwin: goodwin.o demolib.o watcherGPS.o watchermovement.o libidsCommunications.a
	$(CXX) -o goodwin goodwin.o demolib.o watcherGPS.o watchermovement.o -L . -lidsCommunications  $(LDFLAGS)

statetest: statetest.o libidsCommunications.a
	$(CXX) -o statetest statetest.o -L . -lidsCommunications  $(LDFLAGS)

goodwintest: goodwintest.o libidsCommunications.a watcherGPS.o
	$(CXX) -o goodwintest goodwintest.o watcherGPS.o -L . -lidsCommunications  $(LDFLAGS)

goodwinmerge: goodwinmerge.o watcherGPS.o libidsCommunications.a
	$(CXX) -o goodwinmerge goodwinmerge.o watcherGPS.o -L . -lidsCommunications  $(LDFLAGS)

goodwincreate: goodwincreate.o apisupport.o libidsCommunications.a
	$(CXX) -o goodwincreate goodwincreate.o apisupport.o -L . -lidsCommunications  $(LDFLAGS)

demoaggregator: demoaggregator.o demolib.o libidsCommunications.a
	$(CXX) -o demoaggregator demoaggregator.o demolib.o -L . -lidsCommunications  $(LDFLAGS)

demorewriter: demorewriter.o demolib.o libidsCommunications.a
	$(CXX) -o demorewriter demorewriter.o demolib.o -L . -lidsCommunications $(LDFLAGS)

demoresponder: demoresponder.o demolib.o libidsCommunications.a
	$(CXX) -o demoresponder demoresponder.o demolib.o -L . -lidsCommunications  $(LDFLAGS)

demosigningdetector: demosigningdetector.o demolib.o libidsCommunications.a $(PROTECTIONLIB)
	$(CXX) -o demosigningdetector demosigningdetector.o demolib.o -L . -lidsCommunications  $(PROTECTIONLIB) $(LDFLAGS)

demosigningaggregator: demosigningaggregator.o demolib.o libidsCommunications.a $(PROTECTIONLIB)
	$(CXX) -o demosigningaggregator demosigningaggregator.o demolib.o -L . -lidsCommunications  $(PROTECTIONLIB) $(LDFLAGS)

demoverifiedalertreader: demoverifiedalertreader.o demolib.o libidsCommunications.a $(PROTECTIONLIB)
	$(CXX) -o demoverifiedalertreader demoverifiedalertreader.o demolib.o -L . -lidsCommunications  $(PROTECTIONLIB) $(LDFLAGS)

positionweight: positionweight.o libidsCommunications.a
	$(CXX) -o positionweight positionweight.o -L . -lidsCommunications  $(LDFLAGS)

labeltest: labeltest.o libidsCommunications.a
	$(CXX) -o labeltest labeltest.o -L . -lidsCommunications $(LDFLAGS)

watcherpropertytest: watcherpropertytest.o libidsCommunications.a
	$(CXX) -o watcherpropertytest watcherpropertytest.o -L . -lidsCommunications $(LDFLAGS)

watchergraphtest: watchergraphtest.o libidsCommunications.a
	$(CXX) -o watchergraphtest watchergraphtest.o -L . -lidsCommunications $(LDFLAGS)

floatinglabeltest: floatinglabeltest.o libidsCommunications.a
	$(CXX) -o floatinglabeltest floatinglabeltest.o -L . -lidsCommunications $(LDFLAGS)

edgetest: edgetest.o libidsCommunications.a
	$(CXX) -o edgetest edgetest.o -L . -lidsCommunications $(LDFLAGS)

glancer: glancer.o libidsCommunications.a
	$(CXX) -o glancer glancer.o -L . -lidsCommunications $(LDFLAGS)

routingdetector: routingdetector.o libidsCommunications.a
	$(CXX) -o routingdetector routingdetector.o -L . -lidsCommunications $(LDFLAGS)

libidsCommunications.a: ${LIBOBJS}
	$(AR) -r libidsCommunications.a $^
	$(RANLIB) $@

libidsCommunications.so: ${LIBOBJS}
	ld -shared -o libidsCommunications.so $^ 
#	ld -shared -soname libidsCommunications.so -o libidsCommunications.so $^ 

testPacketProtection: packetProtection.cpp packetProtection.h
testPacketProtection: $(PROTECTIONLIB) libidsCommunications.a
testPacketProtection: ${BASEOBJ}
	$(CXX) -o testPacketProtection ${COMPFLAGS} -DTEST_PACKET_PROTECTION packetProtection.cpp ${BASEOBJ} -L. -lidsCommunications $(PROTECTIONLIB) ${LDFLAGS}

# just rebuild the whole thing GTL 
../protection/libnnpprotection.a: 
	$(MAKE) -C ../protection

distclean:
	$(MAKE) clean

clean:
	rm -f $(BINS) *.o *.uto *.core core testhello libidsCommunications.a depend;

install: $(BINS)
#	cp -f $(BINS) $(HOME)/bin

infrasniff: infrasniff.o packetstats.o $(PROTECTIONLIB) $(PACKETPROTECTIONOBJ) $(MODULEOBJ) $(BASEOBJ) libidsCommunications.a
	$(CXX) -o infrasniff infrasniff.o packetstats.o $(PACKETPROTECTIONOBJ) $(MODULEOBJ) $(BASEOBJ) \
		-L . -lidsCommunications $(PROTECTIONLIB) $(LDFLAGS)


#
# Note naming unit test objects .uto instead of .o - this is to get our
# fancy rule that makes position independent code for these objects.
#
ALLUTSRCS:=$(shell (ls ut_*{.c,.cc} 2>/dev/null) | sort)
ALLUTOBJS:=$(ALLUTSRCS:.c=.uto)
%.uto : %.c
	$(CC) -g -c $(CFLAGS) $(CPPFLAGS) $< -o $@
unitTests.so: ${ALLUTOBJS} libidsCommunications.a ../protection/libnnpprotection.a
	ld -L. -L../protection --no-allow-shlib-undefined -shared -soname unitTests.so -o $@ ${ALLUTOBJS} -lidsCommunications -lnnpprotection -lc ${LDSO}

ut: all unitTests.so
	@echo "***" regression test for the enrouteCH spin fixed in interim2.cpp rev 1.83 -- should terminate in a few seconds
	time ./interim2 manet20.deg4.mob1.conf 100 >/dev/null 2>&1
	@echo "***" hcunit tests
	time ../hcunit/hcunit -v ./unitTests.so
	@echo "***" ALL UNIT TESTS SUCCESSFUL "***"

depend: ${ALLSRCS} ${ALLHEADERS}
	@echo makedepend
	@$(RM) depend
	@makedepend -f- -- $(CPPFLAGS) $(CFLAGS) -- ${ALLSRCS} > $@ 2>/dev/null;
	@for f in ${ALLOBJS} depend ; do echo "$$f: Makefile" >> $@; done

# no errors if "depend" doesn't exist
-include depend
