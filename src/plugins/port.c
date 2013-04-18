#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <env.h>
#include <estragon.h>

#define PATHMAX 1024

uv_loop_t* loop;
char* lib_path;

static uv_buf_t port__on_alloc(uv_handle_t* handle, size_t suggested_size) {
  return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void port__on_ipc_read(uv_stream_t* tcp, ssize_t nread, uv_buf_t rdbuf) {
  int port;
  char* str;
  char msg[8];

  if (nread == -1) {
    return;
  }

  str = malloc((nread + 1) * sizeof(char));

  memcpy(str, rdbuf.base, nread);
  if (sscanf(str, "port=%d\n", &port) == 1) {
    sprintf(msg, "%d", port);
    estragon_send("port", "info", msg, 1.0);
  }

  free(str);
  free(rdbuf.base);
}

void port__process_spawned_cb(uv_process_t* process, uv_process_options_t* options) {
  uv_read_start(options->stdio[3].data.stream, port__on_alloc, port__on_ipc_read);
}

void port__process_options_cb(uv_process_options_t* options) {
#if (__APPLE__ && __MACH__)
  options->env = env_set(options->env, "DYLD_FORCE_FLAT_NAMESPACE", "1");
  options->env = env_set(options->env, "DYLD_INSERT_LIBRARIES", lib_path);
#else
  options->env = env_set(options->env, "LD_PRELOAD", lib_path);
#endif
}

int port_init(estragon_plugin_t* plugin) {
  size_t size = PATHMAX / sizeof(*lib_path);

  lib_path = malloc(size);

  loop = uv_default_loop();

  if (uv_exepath(lib_path, &size) != 0) {
    fprintf(stderr, "uv_exepath: %s\n", uv_strerror(uv_last_error(loop)));
    return 1;
  }

#if (__APPLE__ && __MACH__)
  strcpy(strrchr(lib_path, '/') + 1, "libinterposed.dylib");
#else
  strcpy(strrchr(lib_path, '/') + 1, "libinterposed.so");
#endif

  plugin->process_options_cb = port__process_options_cb;
  plugin->process_spawned_cb = port__process_spawned_cb;

  return 0;
}
