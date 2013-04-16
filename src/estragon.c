#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <uv.h>
#include <env.h>
#include <saneopt.h>

#include <estragon.h>
#include <estragon-private/plugins.h>

static uv_loop_t *loop;
extern char** environ;

estragon_plugin_t plugins[PLUGIN_COUNT];
char** arguments;

void on_exit(uv_process_t* process, int exit_status, int term_signal) {
}

void spawn() {
  int i;
  char** env = NULL;

  env = env_copy(environ, env);

  uv_process_t* process = malloc(sizeof(uv_process_t));
  uv_stdio_container_t stdio[3];
  uv_process_options_t options;

  options.stdio_count = 3;
  for (i = 0; i < options.stdio_count; i++) {
    stdio[i].flags = UV_INHERIT_FD;
    stdio[i].data.fd = i;
  }

  options.env = env;
  options.cwd = NULL;
  options.file = arguments[0];
  options.args = arguments;
  options.flags = 0;
  options.stdio = stdio;
  options.exit_cb = on_exit;

  if (uv_spawn(loop, process, options)) {
    fprintf(stderr, "uv_spawn: %s\n", uv_err_name(uv_last_error(loop)));
  }
}

void on_connect(int status) {
  int i;
  if (status) {
    fprintf(stderr, "on_connect: %s\n", uv_strerror(uv_last_error(loop)));
    return;
  }

  printf("connected.\n");

  for (i = 0; i < PLUGIN_COUNT; i++) {
    if (_plugin_init_calls[i](&plugins[i]) != 0) {
      fprintf(stderr, "error initializing plugin %i\n", i);
    }
  }

  //
  // We are connected to the monitoring server. Spawn our new process overlord.
  //
  spawn();
}

int main(int argc, char *argv[]) {
  loop = uv_default_loop();

  printf("estragon "ESTRAGON_VERSION_HASH"\n");

  saneopt_t* opt = saneopt_init(argc - 1, argv + 1);
  saneopt_alias(opt, "port", "p");
  saneopt_alias(opt, "host", "h");
  int port = atoi(saneopt_get(opt, "port"));
  char* host = saneopt_get(opt, "host");
  arguments = saneopt_arguments(opt);

  printf("connecting to %s:%d...\n", host, port);
  estragon_connect(host, port, on_connect);

  uv_run(loop, UV_RUN_DEFAULT);
  return 0;
}
