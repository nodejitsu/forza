#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <uv.h>
#include <saneopt.h>

#include <estragon.h>
#include <estragon-private/plugins.h>

static uv_loop_t *loop;

estragon_plugin_t plugins[PLUGIN_COUNT];

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
}

int main(int argc, char *argv[]) {
  printf("estragon "ESTRAGON_VERSION_HASH"\n");

  saneopt_t* opt = saneopt_init(argc, argv);
  int port = atoi(saneopt_get(opt, "port"));
  char* host = saneopt_get(opt, "host");

  printf("connecting to %s:%d...\n", host, port);
  estragon_connect(host, port, on_connect);

  uv_run(loop, UV_RUN_DEFAULT);
  return 0;
}
