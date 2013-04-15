#include <uv.h>

#ifdef __sun
#include <sys/pset.h>
#include <sys/loadavg.h>
#endif

static uv_timer_t cpu_timer;

void send_cpu_usage(uv_timer_t *timer, int status) {
  double loadinfo[3];
#ifdef DEBUG
  printf("cpu usage timer fired, status %d\n", status);
#endif
#ifdef __sun
  /* On SunOS, if we're not in a global zone, uv_loadavg returns [0, 0, 0] */
  /* This, instead, gets the loadavg for our assigned processor set. */
  pset_getloadavg(PS_MYID, loadinfo, 3);
#else
  uv_loadavg(loadinfo);
#endif
  send_data("CPU load, last minute", "info", loadinfo[0]);
  send_data("CPU load, last 5 minutes", "info", loadinfo[1]);
  send_data("CPU load, last 15 minutes", "info", loadinfo[2]);
}

int cpu_init() {
  uv_timer_init(uv_default_loop(), &cpu_timer);
  uv_timer_start(&cpu_timer, send_cpu_usage, 0, 1000);

  return 0;
}
