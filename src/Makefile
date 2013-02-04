include ../Makefile.conf

all: build

.PHONY: all

clean:
	rm -rf $(CLEAN)

.PHONY: clean

libheca_INC := libheca.h dsm_init.h
libheca_SRC := libheca.c dsm_init.c
libheca_OBJ := $(libheca_SRC:.c=.o)
libheca_LIB := libheca.so
libheca_CFLAGS := $(CFLAGS) -fPIC
libheca_LDFLAGS := $(LDFLAGS) -shared
CLEAN += $(libheca_OBJ) $(libheca_LIB)
TARGET += $(libheca_LIB)

$(libheca_LIB): $(libheca_SRC) $(libheca_INC)
	$(CC) -o $@ $(libheca_SRC) $(libheca_CFLAGS) $(libheca_LDFLAGS) 

master_SRC := master.c
master_OBJ := $(master_SRC:.c=.o)
master_BIN := $(master_SRC:.c=)
master_CFLAGS := $(CFLAGS)
master_LDFLAGS := $(LDFLAGS) -L. -lheca
CLEAN += $(master_OBJ) $(master_BIN)
TARGET += $(master_BIN)

$(master_BIN): $(master_SRC) $(libheca_INC)
	$(CC) -o $@ $(master_SRC) $(master_CFLAGS) $(master_LDFLAGS)

client_SRC := client.c
client_OBJ := $(patsubst %.c,%.o,$(client_SRC))
client_BIN := $(patsubst %.c,%,$(client_SRC))
client_CFLAGS := $(CFLAGS)
client_LDFLAGS := $(LDFLAGS) -L. -lheca
CLEAN += $(client_OBJ) $(client_BIN)
TARGET += $(client_BIN)

$(client_BIN): $(client_SRC) $(libheca_INC)
	$(CC) -o $@ $(client_SRC) $(client_CFLAGS) $(client_LDFLAGS)

build: $(TARGET)

.PHONY: build

install: $(libheca_LIB)
	install $(libheca_INC) /usr/include/
	install $(libheca_LIB) /usr/lib/
	ln -sf /usr/lib/$(libheca_LIB) /usr/lib/$(libheca_LIB).1
	ln -sf /usr/lib/$(libheca_LIB).1 /usr/lib/$(libheca_LIB).1.0
	ln -sf /usr/lib/$(libheca_LIB).1.0 /usr/lib/$(libheca_LIB).1.0.1
	ldconfig -n /usr/lib

.PHONY: install

