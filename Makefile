include config.mk

OBJS += src/forza.o
OBJS += src/connect.o
OBJS += src/json.o

CFLAGS += -g -Wall -Ideps/libuv/include -Ideps/saneopt/include -Ideps/env/include -Iinclude
ifdef FORZA_VERSION_HASH
  CFLAGS += -DFORZA_VERSION_HASH='"$(FORZA_VERSION_HASH)"'
endif

uname_S=$(shell uname -s)

ifeq (Darwin, $(uname_S))
  LDFLAGS += -framework CoreServices
endif

ifeq (Linux, $(uname_S))
  LDFLAGS += -lc -lrt -ldl -lm -lpthread
endif

ifeq (SunOS, $(uname_S))
  LDFLAGS += -lsendfile -lsocket -lkstat -lnsl -lm
endif

all: libuv libsaneopt libenv forza

src/%.o: src/%.c include/forza-private/plugins.h
	gcc $(CFLAGS) -c $< -o $@

src/plugins/%.o: src/plugins/%.c
	gcc $(CFLAGS) -c $< -o $@

src/plugins/start.o: libinterposed

forza: $(OBJS)
	gcc $^ deps/libuv/libuv.a deps/saneopt/libsaneopt.a deps/env/libenv.a $(LDFLAGS) $(CFLAGS) -o $@

libuv:
	$(MAKE) -C deps/libuv/

libsaneopt:
	$(MAKE) -C deps/saneopt/

libenv:
	$(MAKE) -C deps/env/

ifeq (Darwin, $(uname_S))
libinterposed: src/plugins/start/libinterposed.c
	gcc $(INTERPOSED_CFLAGS) $(CFLAGS) -dynamiclib -o libinterposed.dylib $^
else
libinterposed: src/plugins/start/libinterposed.c
	gcc $(INTERPOSED_CFLAGS) $(CFLAGS) -D_GNU_SOURCE -fPIC -shared -o libinterposed.so $^
endif

clean:
	rm -f forza

cleanall:
	rm -f forza
	$(MAKE) clean -C deps/libuv/
	$(MAKE) clean -C deps/saneopt/
	$(MAKE) clean -C deps/env/

test: all
	npm install
	test/run

.PHONY: all test clean cleanall
