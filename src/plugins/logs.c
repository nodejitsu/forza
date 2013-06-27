#include <stdlib.h>
#include <uv.h>
#include <estragon.h>
#include "logs.h"

void logs__on_stdio(char* data, estragon__stdio_type_t type) {
  estragon_metric_t* metric;

  metric = estragon_new_metric();

  metric->metric = 1.0;
  metric->description = data;
  metric->service = (type == STDIO_STDOUT) ? "logs/stdout" : "logs/stderr";

  estragon_send(metric);

  estragon_free_metric(metric);
}

void logs__on_stdout(char* data) {
  logs__on_stdio(data, STDIO_STDOUT);
}

void logs__on_stderr(char* data) {
  logs__on_stdio(data, STDIO_STDERR);
}

int logs_init(estragon_plugin_t* plugin) {
  plugin->stdout_data_cb = logs__on_stdout;
  plugin->stderr_data_cb = logs__on_stderr;

  return 0;
}
