#include <time.h>
#include <uv.h>
#include <forza.h>

static uv_timer_t uptime_timer;
time_t start_time;

void uptime__send_uptime(uv_timer_t *timer, int status) {
  forza_metric_t* metric = forza_new_metric();
  time_t now = time(NULL);

  metric->service = "health/process/uptime";
  metric->metric = (double) (now - start_time);
  forza_send(metric);

  forza_free_metric(metric);
}

void uptime__process_exit_cb(int exit_status, int term_singal) {
  uv_timer_stop(&uptime_timer);
}

int uptime_init(forza_plugin_t* plugin) {
  start_time = time(NULL);

  //
  // Assuming nobody will run this in the last second of New Year's Eve in 1969.
  //
  if (start_time == -1) {
    return -1;
  }

  plugin->process_exit_cb = uptime__process_exit_cb;

  uv_timer_init(uv_default_loop(), &uptime_timer);
  uv_timer_start(&uptime_timer, uptime__send_uptime, 0, 5000);

  return 0;
}
