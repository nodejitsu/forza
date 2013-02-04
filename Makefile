CFLAGS=-g -Wall -Ideps/libuv/include

uname_S=$(shell uname -s)

ifeq (Darwin, $(uname_S))
CFLAGS+=-framework CoreServices
endif

ifeq (Linux, $(uname_S))
CFLAGS+=-lc -lrt -ldl -lm -lpthread
endif

ifeq (SunOS, $(uname_S))
CFLAGS+=-lsendfile -lsocket -lkstat -lnsl -lm
endif

all: libuv godot-agent

godot-agent: godot-agent.c
	gcc $(CFLAGS) -o godot-agent godot-agent.c deps/libuv/libuv.a

libuv: 
	make -C deps/libuv/

clean:
	rm -f godot-agent

cleanall:
	rm -f godot-agent
	make clean -C deps/libuv/
