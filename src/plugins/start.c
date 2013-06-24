#include <env.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <estragon.h>

#define PATHMAX 1024

static char* buffer;
static size_t buffer_len = 0;
static int started = 0;

static uv_timer_t timeout_timer;

uv_loop_t* loop;
char* lib_path;

void start__on_ipc_data(char* data) {
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

void start__process_options_cb(uv_process_options_t* options) {
#if (__APPLE__ && __MACH__)
  options->env = env_set(options->env, "DYLD_FORCE_FLAT_NAMESPACE", "1");
  options->env = env_set(options->env, "DYLD_INSERT_LIBRARIES", lib_path);
#else
  options->env = env_set(options->env, "LD_PRELOAD", lib_path);
#endif
}

void start__failure() {
  estragon_metric_t* metric = estragon_new_metric();
  metric->service = "health/process/start";
  metric->metric = 0.0;
  metric->description = buffer;
  estragon_send(metric);
  estragon_free_metric(metric);
}

void start__success() {
  estragon_metric_t* metric = estragon_new_metric();

  started = 1;

  metric->service = "health/process/start";
  metric->metric = 1.0;
  estragon_send(metric);
  estragon_free_metric(metric);
}

void start__timeout(uv_timer_t* timer, int status) {
  start__failure();
}

void start__on_data(char* data, estragon__stdio_type_t type) {
  size_t len = strlen(data);

  buffer_len += len;
  buffer = realloc(buffer, buffer_len + 1);
  strncat(buffer, data, len);
}

void start__on_stdout_data(char* data) {
  start__on_data(data, STDIO_STDOUT);
}

void start__on_stderr_data(char* data) {
  start__on_data(data, STDIO_STDERR);
}

void start__process_spawned_cb(uv_process_t* process, uv_process_options_t* options) {
  uv_timer_init(uv_default_loop(), &timeout_timer);
  uv_timer_start(&timeout_timer, start__timeout, 15000, 0);
}

void start__process_exit_cb(int exit_status, int term_signal) {
  if (!started) {
    uv_timer_stop(&timeout_timer);
    start__failure();
  }
}

int start_init(estragon_plugin_t* plugin) {
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

  plugin->process_options_cb = start__process_options_cb;
  plugin->ipc_data_cb = start__on_ipc_data;
  plugin->process_spawned_cb = start__process_spawned_cb;
  plugin->process_exit_cb = start__process_exit_cb;
  plugin->stdout_data_cb = start__on_stdout_data;
  plugin->stderr_data_cb = start__on_stderr_data;


  return 0;
}
