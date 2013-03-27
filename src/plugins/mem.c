#include "../../deps/libuv/include/uv.h"

static uv_timer_t mem_timer;

void send_mem_usage(uv_timer_t *timer, int status) {
  double mempct;
  uint64_t freemem = uv_get_free_memory();
  uint64_t totalmem = uv_get_total_memory();

#ifdef DEBUG
  printf("memory usage timer fired, status %d\n", status);
#endif
  mempct = (double)(totalmem - freemem) / (double)totalmem;
  send_data("Memory Usage (%)", "info", mempct);
}

int mem_init() {
  uv_timer_init(uv_default_loop(), &mem_timer);
  uv_timer_start(&mem_timer, send_mem_usage, 0, 1000);

  return 0;
}
