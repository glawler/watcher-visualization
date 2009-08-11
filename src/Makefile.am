ACLOCAL_AMFLAGS=-I aclocal

include $(srcdir)/Makefile.top

if !ENABLE_TESTNODEONLY
FULL_BUILD_DIRS = sqlite_wrapper watcherd 
endif

if ENABLE_HIERARCHY_CLIENT
IDS_COMMS_DIR = idsCommunications
endif

SUBDIRS=logger util libwatcher clients $(FULL_BUILD_DIRS) $(IDS_COMMS_DIR)

.PHONY: doc

doc:
	doxygen Doxyfile
	$(MAKE) -C doc
