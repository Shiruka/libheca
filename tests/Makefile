include ../Makefile.conf

libheca_DIR := $(CURDIR)/../src

tst_SRC := tst.c config.c dsm.c
tst_INC := config.h dsm.h tst.h
tst_BIN := tst
tst_CFLAGS := $(CFLAGS) -I. -I$(libheca_DIR)
tst_LDFLAGS := $(LDFLAGS) -L$(libheca_DIR) -lheca -lm

poc_SRC := poc.c quicksort.c config.c dsm.c
poc_INC := quicksort.h config.h dsm.h poc.h
poc_BIN := poc
poc_CFLAGS := $(CFLAGS) -I. -I$(libheca_DIR)
poc_LDFLAGS := $(LDFLAGS) -L$(libheca_DIR) -lheca -lm

cctst_SRC := cc-tst.c config.c dsm.c
cctst_INC := config.h dsm.h cc-tst.h
cctst_BIN := cc-tst
cctst_CFLAGS := $(CFLAGS) -I. -I$(libheca_DIR)
cctst_LDFLAGS := $(LDFLAGS) -L$(libheca_DIR) -lheca -lm

ccpoc_SRC := cc-poc.c config.c dsm.c
ccpoc_INC := config.h dsm.h cc-poc.h
ccpoc_BIN := cc-poc
ccpoc_CFLAGS := $(CFLAGS) -I. -I$(libheca_DIR)
ccpoc_LDFLAGS := $(LDFLAGS) -L$(libheca_DIR) -lheca -lm

all: $(tst_BIN) $(poc_BIN) $(ccpoc_BIN) $(cctst_BIN)

.PHONY: all

install:

.PHONY: install

$(tst_BIN): $(tst_SRC) $(tst_INC)
	$(CC) $(tst_CFLAGS) $(tst_SRC) -o $@ $(tst_LDFLAGS)

CLEAN += $(tst_BIN)

$(poc_BIN): $(poc_SRC) $(poc_INC)
	$(CC) $(poc_CFLAGS) $(poc_SRC) -o $@ $(poc_LDFLAGS)

CLEAN += $(poc_BIN)

$(cctst_BIN): $(cctst_SRC) $(cctst_INC)
	$(CC) $(cctst_CFLAGS) $(cctst_SRC) -o $@ $(cctst_LDFLAGS)

CLEAN += $(cctst_BIN)

$(ccpoc_BIN): $(ccpoc_SRC) $(ccpoc_INC)
	$(CC) $(ccpoc_CFLAGS) $(ccpoc_SRC) -o $@ $(ccpoc_LDFLAGS)

CLEAN += $(ccpoc_BIN)

clean:
	rm -rf $(CLEAN)

.PHONY: clean

