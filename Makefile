SUBDIRS = src tests

all install clean:
	for i in $(SUBDIRS); do \
		make -C $$i $@; \
	done

.PHONY: all clean
