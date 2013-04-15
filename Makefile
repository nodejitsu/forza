-include config.mk

OBJS += src/estragon.o
OBJS += src/options.o

CFLAGS=-g -Wall -Ideps/libuv/include -Ideps/saneopt/include -Iinclude
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

src/%.o: src/%.c
	gcc $(CFLAGS) -c $< -o $@

src/plugins/%.o: src/plugins/%.c
	gcc $(CFLAGS) -c $< -o $@

estragon: $(OBJS)
	gcc $(LDFLAGS) deps/libuv/libuv.a deps/saneopt/libsaneopt.a deps/env/libenv.a $^ -o $@

libuv:
	make -C deps/libuv/

libsaneopt:
	make -C deps/saneopt/

libenv:
	make -C deps/env/

clean:
	rm -f estragon

cleanall:
	rm -f estragon
	make clean -C deps/libuv/
	make clean -C deps/saneopt/
	make clean -C deps/env/
