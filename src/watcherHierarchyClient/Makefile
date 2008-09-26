TOPDIR=../..
WATCHDIR=$(TOPDIR)/src/watcherd

CFLAGS+=-I$(TOPDIR)/include 
CXXFLAGS+=-I$(TOPDIR)/include -I/usr/local/include/boost-1_36 -I$(WATCHDIR) -O0 -ggdb

LDFLAGS+=-L $(TOPDIR)/lib -L/usr/local/lib

LIBS+=-lidsCommunications -lxml2 -llog4cxx -llogger
LIBS+=-lboost_serialization-gcc41-mt
LIBS+=-lboost_system-gcc41-mt

OBJS=watcherHierarchyClient.o demolib.o \
	$(WATCHDIR)/labelMessage.o \
	$(WATCHDIR)/message.o \
	$(WATCHDIR)/labelMessage.o \
	$(WATCHDIR)/watcherColors.o \
	$(WATCHDIR)/messageTypesAndVersions.o \
	$(WATCHDIR)/watcherGlobalFunctions.o \
	$(WATCHDIR)/client.o \
	$(WATCHDIR)/clientConnection.o \
	$(WATCHDIR)/dataMarshaller.o \
	$(WATCHDIR)/labelMessage.o \
	$(WATCHDIR)/edgeMessage.o \
	$(WATCHDIR)/messageStatus.o \
	legacyWatcherMessageUnmarshal.o

watcherHierarchyClient: $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LIBS)

tags: 
	ctags -R . $(TOPDIR)/src/watcherd $(TOPDIR)/include 

depend: $(SRCS)
	@echo makedepend
	@$(RM) depend
	@makedepend -f- *.cpp *.h ../*.cpp ../*.h* > $@ 2>/dev/null;
	@for f in ${OBJS} depend ; do echo "$$f: Makefile" >> $@; done

clean:
	rm -f $(OBJS) watcherHierarchyClient

DEPEND=$(shell if [ -r depend ] ; then echo -n 1; else echo -n 0; fi;)
ifeq (${DEPEND}, 1)
	include depend
endif



