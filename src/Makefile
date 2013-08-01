include ../Makefile.conf

all: build

.PHONY: all

clean:
	rm -rf $(CLEAN)

.PHONY: clean

libheca_INC := libheca.h socket.h ioctl.h
libheca_SRC := libheca.c socket.c ioctl.c
libheca_OBJ := $(libheca_SRC:.c=.o)
libheca_LIB := libheca.so
libheca_CFLAGS := $(CFLAGS) -fPIC
libheca_LDFLAGS := $(LDFLAGS) -shared
CLEAN += $(libheca_OBJ) $(libheca_LIB)
TARGET += $(libheca_LIB)

$(libheca_LIB): $(libheca_SRC) $(libheca_INC)
	$(CC) -o $@ $(libheca_SRC) $(libheca_CFLAGS) $(libheca_LDFLAGS) 

build: $(TARGET)

.PHONY: build

install: $(libheca_LIB) $(libheca_INC)
	install $(libheca_INC) /usr/include/
	install $(libheca_LIB) /usr/lib/
	ln -sf /usr/lib/$(libheca_LIB) /usr/lib/$(libheca_LIB).1
	ln -sf /usr/lib/$(libheca_LIB).1 /usr/lib/$(libheca_LIB).1.0
	ln -sf /usr/lib/$(libheca_LIB).1.0 /usr/lib/$(libheca_LIB).1.0.1
	ldconfig -n /usr/lib

.PHONY: install

