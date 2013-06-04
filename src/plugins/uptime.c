#include <time.h>
#include <uv.h>
#include <estragon.h>

static uv_timer_t uptime_timer;
time_t start_time;

void uptime__send_uptime(uv_timer_t *timer, int status) {
  estragon_metric_t* metric = estragon_new_metric();
  time_t now = time(NULL);

  metric->service = "health/process/uptime";
  metric->metric = (double) (now - start_time);
  estragon_send(metric);

  estragon_free_metric(metric);
}

int uptime_init() {
  start_time = time(NULL);

  //
  // Assuming nobody will run this in the last second of New Year's Eve in 1969.
  //
  if (start_time == -1) {
    return -1;
  }

  uv_timer_init(uv_default_loop(), &uptime_timer);
  uv_timer_start(&uptime_timer, uptime__send_uptime, 0, 5000);

  return 0;
}
