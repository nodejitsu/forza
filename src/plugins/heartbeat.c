#include <uv.h>
#include <estragon.h>

static uv_timer_t heartbeat_timer;

void heartbeat__send(uv_timer_t *timer, int status) {
  estragon_metric_t* metric = estragon_new_metric();

  //
  // This is probably more accurately machine but this makes it
  // 1000% easier to aggregate
  //
  metric->service = "health/process/heartbeat";
  metric->metric = 1;
  estragon_send(metric);

  estragon_free_metric(metric);
}

int heartbeat_init(estragon_plugin_t* plugin) {
  uv_timer_init(uv_default_loop(), &heartbeat_timer);
  uv_timer_start(&heartbeat_timer, heartbeat__send, 0, 90000);

  return 0;
}
