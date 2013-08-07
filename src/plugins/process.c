#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <env.h>
#include <forza.h>

void process__process_spawned_cb(uv_process_t* process, uv_process_options_t* options) {
  forza_metric_t* metric = forza_new_metric();
  metric->metric = 1.0;
  metric->service = "health/process/spawn";
  metric->meta->pid = process->pid;
  forza_send(metric);
  forza_free_metric(metric);
}

void process__process_exit_cb(int exit_status, int term_singal) {
  char msg[16];
  forza_metric_t* metric = forza_new_metric();

  sprintf(msg, "%d", exit_status);

  metric->metric = 1.0;
  metric->service = "health/process/exit";
  metric->description = msg;

  forza_send(metric);
  forza_free_metric(metric);
}

int process_init(forza_plugin_t* plugin) {
  plugin->process_spawned_cb = process__process_spawned_cb;
  plugin->process_exit_cb = process__process_exit_cb;

  return 0;
}
