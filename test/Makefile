CC := gcc
SRC := poc.c tst.c
BIN := $(patsubst %.c,%,$(SRC))

DEPINC :=
DEPLIB := lib/quicksort.c lib/config.c lib/dsm.c
DEPINC := headers/quicksort.h headers/config.h headers/dsm.h
DEPBIN := $(patsubst %.c,%,$(DEPLIB))

CFLAGS := -g -Wall
LDFLAGS := -lm -lpthread -lheca

%:
	$(CC) $(CFLAGS) $(DEPLIB) -L. $@.c -o $@ $(LDFLAGS)

all: clean $(BIN)

clean:
	rm -rf $(BIN) $(DEPBIN)

.PHONY: all clean
