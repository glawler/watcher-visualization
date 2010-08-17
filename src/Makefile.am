ACLOCAL_AMFLAGS=-I aclocal

include $(srcdir)/Makefile.top

if !ENABLE_TESTNODEONLY
FULL_BUILD_DIRS = sqlite_wrapper watcherd 
endif

SUBDIRS=util libwatcher $(FULL_BUILD_DIRS) clients

.PHONY: doc

doc:
	doxygen Doxyfile
	$(MAKE) -C doc

# make install into staging dir, uninstall "server" components by hand, 
# then tar it up and remove the tmp dir.
TMPDISTDIR = $(shell pwd)/tmpDistDir
testnode-dist:
	@echo Temporarily installing distribution
	mkdir -p $(TMPDISTDIR) && $(MAKE) DESTDIR=$(TMPDISTDIR) install
	@echo Removing non-testnode files from distribution
	$(RM) $(TMPDISTDIR)/$(bindir)/showClock
	$(RM) $(TMPDISTDIR)/$(bindir)/messageStream2Text
	$(RM) $(TMPDISTDIR)/$(bindir)/connectivity2dot
	$(RM) $(TMPDISTDIR)/$(bindir)/watcherd
	$(RM) $(TMPDISTDIR)/$(bindir)/ogreWatcher
	$(RM) $(TMPDISTDIR)/$(bindir)/randomScenario
	@echo Creating $(top_srcdir)/testnode-dist.tbz tar file.
	tar jcvf $(top_srcdir)/testnode-dist.tbz -C $(TMPDISTDIR) .
	@echo Removing tmp installation directory
	$(RM) -r $(TMPDISTDIR)
	@echo testnode-dist complete, testnode-dist is $(top_srcdir)/testnode-dist.tbz

# make install into staging dir, uninstall "testnode" components by hand, 
# then tar it up and remove the tmp dir.
server-dist:
	@echo Temporarily installing distribution
	mkdir -p $(TMPDISTDIR) && $(MAKE) DESTDIR=$(TMPDISTDIR) install
	@echo Copying legacy watcher GUI if it is there.
	if [ -e $(top_srcdir)/clients/legacyWatcher/watcher ]; then cp -v $(top_srcdir)/clients/legacyWatcher/watcher $(TMPDISTDIR)/$(bindir); fi;
	@echo Removing non-servernode files from distribution
	$(RM) $(TMPDISTDIR)/$(bindir)/routeFeeder
	@echo Creating $(top_srcdir)/server-dist.tbz tar file.
	tar jcvf $(top_srcdir)/server-dist.tbz -C $(TMPDISTDIR) .
	@echo Removing tmp installation directory
	$(RM) -r $(TMPDISTDIR)
	@echo server-dist complete, server-dist is $(top_srcdir)/server-dist.tbz


