include ../Makefile.conf

libheca_DIR := $(CURDIR)/../src
DEP := config.c dsm.c
INC := config.h dsm.h
LDFLAGS := $(LDFLAGS) -L$(libheca_DIR) -lheca -lm
CFLAGS := $(CFLAGS) -I. -I$(libheca_DIR)

#
# To add a new test:
#   1. Create {name}.c and {name}.h 
#   2. Add the {name}_BIN, {name}_DEP and {name}_INC entries. 
#   3. List it in BIN.
#
tst_BIN := tst
tst_DEP :=
tst_INC :=

poc_BIN := poc
poc_DEP := quicksort.c
poc_INC := quicksort.h

cctst_BIN := cc-tst
cctst_DEP :=
cctst_INC :=

ccpoc_BIN := cc-poc
ccpoc_DEP := 
ccpoc_INC :=

BIN := $(tst_BIN) $(poc_BIN) $(cctst_BIN) $(ccpoc_BIN)

%: %.c $(DEP) $(value %_DEP) %.h $(INC) $(value %_INC)
	$(CC) $(CFLAGS) $@.c $(DEP) $(value $@_DEP) -o $@ $(LDFLAGS)

.PHONY: all
all: $(BIN)

.PHONY: install
install:

.PHONY: clean
clean:
	rm -rf $(BIN)

