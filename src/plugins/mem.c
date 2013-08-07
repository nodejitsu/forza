#include <uv.h>
#include <forza.h>

static uv_timer_t mem_timer;

void mem__send_usage(uv_timer_t *timer, int status) {
  double mempct;
  uint64_t freemem = uv_get_free_memory();
  uint64_t totalmem = uv_get_total_memory();

#ifdef DEBUG
  printf("memory usage timer fired, status %d\n", status);
#endif
  mempct = (double)(totalmem - freemem) / (double)totalmem;

  forza_metric_t* metric = forza_new_metric();
  metric->service = "health/machine/memory";
  metric->metric = mempct;
  forza_send(metric);
  forza_free_metric(metric);
}

void mem__process_exit_cb(int exit_status, int term_singal) {
  uv_timer_stop(&mem_timer);
}

int mem_init(forza_plugin_t* plugin) {
  plugin->process_exit_cb = mem__process_exit_cb;

  uv_timer_init(uv_default_loop(), &mem_timer);
  uv_timer_start(&mem_timer, mem__send_usage, 0, 5000);

  return 0;
}
