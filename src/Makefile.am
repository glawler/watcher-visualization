ACLOCAL_AMFLAGS=-I aclocal

include $(srcdir)/Makefile.top

SUBDIRS=logger util sqlite_wrapper libwatcher watcherd clients

.PHONY: doc

doc:
	doxygen Doxyfile
