#include <stdlib.h>
#include <uv.h>
#include <forza.h>
#include "logs.h"

void logs__on_stdio(char* data, forza__stdio_type_t type) {
  forza_metric_t* metric;

  metric = forza_new_metric();

  metric->metric = 1.0;
  metric->description = data;
  metric->service = (type == STDIO_STDOUT) ? "logs/stdout" : "logs/stderr";

  forza_send(metric);

  forza_free_metric(metric);
}

void logs__on_stdout(char* data) {
  logs__on_stdio(data, STDIO_STDOUT);
}

void logs__on_stderr(char* data) {
  logs__on_stdio(data, STDIO_STDERR);
}

int logs_init(forza_plugin_t* plugin) {
  plugin->stdout_data_cb = logs__on_stdout;
  plugin->stderr_data_cb = logs__on_stderr;

  return 0;
}
