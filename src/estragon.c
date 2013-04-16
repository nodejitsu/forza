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
