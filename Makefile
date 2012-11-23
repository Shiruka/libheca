SRC := master.c client.c
BIN := $(patsubst %.c,%,$(SRC))
DEBINC := libheca.h dsm_init.h
DEPLIB := libheca.c dsm_init.c
DEPBIN := $(patsubst %.c,%,$(DEPLIB))

CFLAGS := -g -Wall -pthread $(DEPLIB) -DDEBUG

%: %c %(DEPINC)
	gcc $(CFLAGS) $@ $^

all: clean $(BIN)

clean:
	rm -rf $(BIN) $(DEPBIN)

libheca:
	rm -f *.o *.so
	gcc -g -Wall -fPIC -DDEBUG -c dsm_init.c
	gcc -g -Wall -fPIC -DDEBUG -c libheca.c
	gcc -g -Wall -shared -o libheca.so libheca.o dsm_init.o -lpthread -DDEBUG
	cp libheca.h /usr/include/libheca.h
	cp libheca.so /usr/lib/libheca.so.1.0.1
	ln -sf /usr/lib/libheca.so.1.0.1 /usr/lib/libheca.so.1.0
	ln -sf /usr/lib/libheca.so.1.0 /usr/lib/libheca.so.1
	ln -sf /usr/lib/libheca.so.1 /usr/lib/libheca.so
	ldconfig -n /usr/lib

libheca_master: 
	gcc -g -Wall -L. master.c -lheca -o master

libheca_client:
	gcc -g -Wall -L. client.c -lheca -o client

libheca_old_way:
	gcc -g -Wall -fPIC -DDEBUG -c libheca.c dsm_init.c
	ld -shared -soname libheca.so.1 -o libheca.so.1.0 -lc libheca.o dsm_init.o
	ldconfig -v -n .
	ln -sf libheca.so.1 libheca.so
	gcc -g -Wall -shared -o libheca.so libheca.o dsm_init.o -lpthread -DDEBUG

