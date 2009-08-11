ACLOCAL_AMFLAGS=-I aclocal

include $(srcdir)/Makefile.top

SUBDIRS=logger util libwatcher clients

if !ENABLE_TESTNODEONLY
	SUBDIRS += sqlite_wrapper watcherd 
endif

if ENABLE_HIERARCHY_CLIENT
	SUBDIRS += idsCommunications
endif

.PHONY: doc

doc:
	doxygen Doxyfile
	$(MAKE) -C doc
