#include "../../deps/libuv/include/uv.h"

static uv_timer_t heartbeat_timer;

void send_heartbeat(uv_timer_t *timer, int status) {
#ifdef DEBUG
  printf("heartbeat timer fired, status %d\n", status);
#endif
  send_data("heartbeat", "info", 0);
}

int heartbeat_init() {
  uv_timer_init(uv_default_loop(), &heartbeat_timer);
  uv_timer_start(&heartbeat_timer, send_heartbeat, 0, 1000);

  return 0;
}
