#include <uv.h>
#include <estragon.h>

#ifdef __sun
#include <sys/pset.h>
#include <sys/loadavg.h>
#endif

static uv_timer_t cpu_timer;

void cpu__send_usage(uv_timer_t *timer, int status) {
  double loadinfo[3];
  estragon_metric_t* metric = estragon_new_metric();

#ifdef DEBUG
  printf("cpu usage timer fired, status %d\n", status);
#endif
#ifdef __sun
  /* On SunOS, if we're not in a global zone, uv_loadavg returns [0, 0, 0] */
  /* This, instead, gets the loadavg for our assigned processor set. */
  pset_getloadavg(PS_MYID, loadinfo, 1);
#else
  uv_loadavg(loadinfo);
#endif

  metric->service = "health/machine/cpu";
  metric->metric = loadinfo[0];
  estragon_send(metric);

  estragon_free_metric(metric);
}

int cpu_init() {
  uv_timer_init(uv_default_loop(), &cpu_timer);
  uv_timer_start(&cpu_timer, cpu__send_usage, 0, 5000);

  return 0;
}
