#include <uv.h>
#include <forza.h>
#include <saneopt.h>

#ifdef __sun
#include <sys/pset.h>
#include <sys/loadavg.h>
#endif

static char* user;
static char* name;
static uv_timer_t cpu_timer;

void cpu__send_usage(uv_timer_t *timer, int status) {
  double loadinfo[3];
  forza_metric_t* metric = forza_new_metric();

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
  metric->meta->app->user = user;
  metric->meta->app->name = name;

  forza_send(metric);

  forza_free_metric(metric);
}

void cpu__process_exit_cb(int exit_status, int term_signal) {
  uv_timer_stop(&cpu_timer);
}

int cpu_init(forza_plugin_t* plugin) {
  plugin->process_exit_cb = cpu__process_exit_cb;

  uv_timer_init(uv_default_loop(), &cpu_timer);
  uv_timer_start(&cpu_timer, cpu__send_usage, 0, 5000);

  user = saneopt_get(plugin->saneopt, "app-user");
  name = saneopt_get(plugin->saneopt, "app-name");

  return 0;
}
