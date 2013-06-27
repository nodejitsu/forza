-include config.mk

OBJS += src/estragon.o
OBJS += src/connect.o
OBJS += src/json.o

CFLAGS=-g -Wall -Ideps/libuv/include -Ideps/saneopt/include -Ideps/env/include -Iinclude
CFLAGS += -DESTRAGON_VERSION_HASH='"$(ESTRAGON_VERSION_HASH)"'

uname_S=$(shell uname -s)

ifeq (Darwin, $(uname_S))
  LDFLAGS=-framework CoreServices
endif

ifeq (Linux, $(uname_S))
  LDFLAGS=-lc -lrt -ldl -lm -lpthread
endif

ifeq (SunOS, $(uname_S))
  LDFLAGS=-lsendfile -lsocket -lkstat -lnsl -lm
endif

all: libuv libsaneopt libenv estragon

src/%.o: src/%.c include/estragon-private/plugins.h
	gcc $(CFLAGS) -c $< -o $@

src/plugins/%.o: src/plugins/%.c
	gcc $(CFLAGS) -c $< -o $@

src/plugins/start.o: libinterposed

estragon: $(OBJS)
	gcc $^ deps/libuv/libuv.a deps/saneopt/libsaneopt.a deps/env/libenv.a $(LDFLAGS) -o $@

libuv:
	$(MAKE) -C deps/libuv/

libsaneopt:
	$(MAKE) -C deps/saneopt/

libenv:
	$(MAKE) -C deps/env/

ifeq (Darwin, $(uname_S))
libinterposed: src/plugins/start/libinterposed.c
	gcc -dynamiclib -o libinterposed.dylib $^
else
libinterposed: src/plugins/start/libinterposed.c
	gcc $(CFLAGS) -D_GNU_SOURCE -fPIC -shared -o libinterposed.so $^
endif

clean:
	rm -f estragon

cleanall:
	rm -f estragon
	$(MAKE) clean -C deps/libuv/
	$(MAKE) clean -C deps/saneopt/
	$(MAKE) clean -C deps/env/

test: all
	npm install
	test/run

.PHONY: all test clean cleanall
