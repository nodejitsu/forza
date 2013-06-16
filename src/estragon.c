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

//
// Pipes for child process I/O.
//
static uv_pipe_t child_stdout;
static uv_pipe_t child_stderr;
static uv_pipe_t child_ipc;

void on_process_exit(uv_process_t* process, int exit_status, int term_signal) {
  int i;

  for (i = 0; i < PLUGIN_COUNT; i++) {
    if (plugins[i].process_exit_cb) {
      plugins[i].process_exit_cb(exit_status, term_signal);
    }
  }

  estragon_close();
  uv_close((uv_handle_t*) process, NULL);
}

void spawn() {
  int i;
  char** env = NULL;

  env = env_copy(environ, env);

  uv_process_t* process = malloc(sizeof(uv_process_t));
  uv_stdio_container_t stdio[4];
  uv_process_options_t options;

  uv_pipe_init(loop, &child_stdout, 0);
  uv_pipe_init(loop, &child_stderr, 0);
  uv_pipe_init(loop, &child_ipc, 0);

  //
  // Setup child's stdio. stdout and stderr are pipes so that we can read
  // child process' output.
  // FD 3 is a pipe used for IPC.
  //
  options.stdio_count = 4;
  stdio[0].flags = UV_INHERIT_FD;
  stdio[0].data.fd = 0;
  stdio[1].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
  stdio[1].data.stream = (uv_stream_t*) &child_stdout;
  stdio[2].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
  stdio[2].data.stream = (uv_stream_t*) &child_stderr;
  stdio[3].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
  stdio[3].data.stream = (uv_stream_t*) &child_ipc;

  options.env = env;
  options.cwd = NULL;
  options.file = arguments[0];
  options.args = arguments;
  options.flags = 0;
  options.stdio = stdio;
  options.exit_cb = on_process_exit;

  for (i = 0; i < PLUGIN_COUNT; i++) {
    if (plugins[i].process_options_cb) {
      plugins[i].process_options_cb(&options);
    }
  }

  if (uv_spawn(loop, process, options)) {
    fprintf(stderr, "uv_spawn: %s\n", uv_err_name(uv_last_error(loop)));
    return;
  }

  for (i = 0; i < PLUGIN_COUNT; i++) {
    if (plugins[i].process_spawned_cb) {
      plugins[i].process_spawned_cb(process, &options);
    }
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
  char** hosts;
  char* hostname;
  int i, c = 0;
  uv_interface_address_t* addresses;
  uv_err_t err;

  loop = uv_default_loop();

  printf("estragon "ESTRAGON_VERSION_HASH"\n");

  saneopt_t* opt = saneopt_init(argc - 1, argv + 1);
  saneopt_alias(opt, "host", "h");
  hosts = saneopt_get_all(opt, "host");
  hostname = saneopt_get(opt, "hostname");
  arguments = saneopt_arguments(opt);

  if (hostname == NULL) {
    hostname = malloc(256 * sizeof(*hostname));
    err = uv_interface_addresses(&addresses, &c);
    if (err.code != UV_OK) {
      fprintf(stderr, "uv_interface_addresses: %s\n", uv_err_name(uv_last_error(loop)));
      return 1;
    }
    for (i = 0; i < c; i++) {
      /* For now, only grab the first non-internal, non 0.0.0.0 interface.
       * TODO: Make this smarter.
       */
      if (addresses[i].is_internal) continue;
      uv_ip4_name(&addresses[i].address.address4, hostname, sizeof(hostname));
      if (strcmp(hostname, "0.0.0.0") != 0) break;
    }
    uv_free_interface_addresses(addresses, c);
  }

  estragon_connect(hosts, hostname, on_connect);

  uv_run(loop, UV_RUN_DEFAULT);

  free(hosts);

  return 0;
}
