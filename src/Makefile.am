ACLOCAL_AMFLAGS=-I aclocal

include $(srcdir)/Makefile.top

if !ENABLE_TESTNODEONLY
FULL_BUILD_DIRS = sqlite_wrapper watcherd 
endif

SUBDIRS=util libwatcher clients $(FULL_BUILD_DIRS) 

.PHONY: doc

doc:
	doxygen Doxyfile
	$(MAKE) -C doc
