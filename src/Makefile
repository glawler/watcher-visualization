SUBDIRS=logger util libwatcher watcherd

.PHONY: all clean

all:
	for i in ${SUBDIRS}; do make -C $$i; done

clean:
	for i in ${SUBDIRS}; do make -C $$i $@; done
