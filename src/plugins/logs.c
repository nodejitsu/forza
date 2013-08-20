#include <stdlib.h>
#include <uv.h>
#include <forza.h>
#include <saneopt.h>
#include "logs.h"

static char* user;
static char* name;

void logs__on_stdio(char* data, forza__stdio_type_t type) {
  forza_metric_t* metric;

  metric = forza_new_metric();

  metric->metric = 1.0;
  metric->description = data;
  metric->service = (type == STDIO_STDOUT) ? "logs/stdout" : "logs/stderr";
  metric->meta->app->user = user;
  metric->meta->app->name = name;

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

  user = saneopt_get(plugin->saneopt, "app-user");
  name = saneopt_get(plugin->saneopt, "app-name");

  return 0;
}
