#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <env.h>
#include <estragon.h>

#define PATHMAX 1024

uv_loop_t* loop;
char* lib_path;

void port__on_ipc_data(char* data) {
  int port;
  char msg[8];
  estragon_metric_t* metric;

  if (sscanf(data, "port=%d\n", &port) == 1) {
    sprintf(msg, "%d", port);

    metric = estragon_new_metric();
    metric->metric = 1.0;
    metric->description = msg;
    metric->service = "port";

    estragon_send(metric);

    estragon_free_metric(metric);
  }
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
  plugin->ipc_data_cb = port__on_ipc_data;

  return 0;
}
