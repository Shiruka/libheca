include ../Makefile.conf

libheca_DIR := ${PWD}/../src

tst_SRC := tst.c config.c dsm.c
tst_INC := config.h dsm.h
tst_BIN := tst
tst_CFLAGS := $(CFLAGS) -I. -I$(libheca_DIR)
tst_LDFLAGS := $(LDFLAGS) -L$(libheca_DIR) -lheca -lm

poc_SRC := poc.c quicksort.c config.c dsm.c
poc_INC := quicksort.h config.h dsm.h
poc_BIN := poc
poc_CFLAGS := $(CFLAGS) -I. -I$(libheca_DIR)
poc_LDFLAGS := $(LDFLAGS) -L$(libheca_DIR) -lheca -lm

all: $(tst_BIN) $(poc_BIN)

.PHONY: all

install:

.PHONY: install

$(tst_BIN): $(tst_SRC) $(tst_INC)
	$(CC) $(tst_CFLAGS) $(tst_SRC) -o $@ $(tst_LDFLAGS)

CLEAN += $(tst_BIN)

$(poc_BIN): $(poc_SRC) $(poc_INC)
	$(CC) $(poc_CFLAGS) $(poc_SRC) -o $@ $(poc_LDFLAGS)

CLEAN += $(poc_BIN)

clean:
	rm -rf $(CLEAN)

.PHONY: clean

