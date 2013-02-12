#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../deps/libuv/include/uv.h"
#include "options.h"

#ifndef PLUGIN_INIT_CALLS
  #define PLUGIN_INIT_CALLS
#endif

#define INITIALIZE_PLUGINS() do { PLUGIN_INIT_CALLS } while(0)

static uv_loop_t *loop;
static uv_connect_t connect_req;
static uv_tcp_t client;
static char hostname[512];
static options_t opts;

static uv_timer_t mem_timer;

char* make_json(char *name, char *state, double value) {
  char *json_buf;
  /* We PROBABLY won't ever need to send more than 1kb of JSON at one time. */
  /* TODO: Revisit this decision. */
  json_buf = malloc(sizeof(char) * 1024);
  snprintf(json_buf, 1024, 
      "{"
      "\"host\":\"%s\","
      "\"service\":\"%s\","
      "\"state\":\"%s\","
      "\"time\":\"%llu\","
      "\"metric\":\"%f\","
      "\"ttl\":\"15\""
      "}\n", hostname, name, state, uv_hrtime(), value);
  return json_buf;
}

void after_write(uv_write_t *req, int status) {
  if (status) {
    fprintf(stderr, "uv_write error: %s\n", uv_strerror(uv_last_error(loop)));
  } 
  free(req);
}

void send_data(char *name, char *state, double value) {
  int r;
  uv_buf_t send_buf;
  uv_stream_t *stream;
  uv_write_t *write_req;
  char *json_data = make_json(name, state, value);

  write_req = malloc(sizeof *write_req);
  send_buf = uv_buf_init(json_data, sizeof(char) * strlen(json_data));

  stream = connect_req.handle;

  r = uv_write(write_req, stream, &send_buf, 1, after_write);
  if (r) {
    fprintf(stderr, "uv_write: %s\n", uv_strerror(uv_last_error(loop)));
    return;
  }
  free(json_data);
}



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

void on_connect(uv_connect_t *req, int status) {
  if (status) {
    fprintf(stderr, "on_connect: %s\n", uv_strerror(uv_last_error(loop)));
    return;
  }
#ifdef DEBUG
  printf("Successfully connected!\n");
#endif

  /* Setup timers for heartbeat and resource reporting */
  uv_timer_init(loop, &mem_timer);

  uv_timer_start(&mem_timer, send_mem_usage, 0, opts.interval);

  INITIALIZE_PLUGINS();
}

int main(int argc, char *argv[]) {
  printf("estragon "ESTRAGON_VERSION_HASH"\n");

  opts = options_parse(argc, argv);
  loop = uv_default_loop();
  /* Get the hostname so that it can be provided to the server */
  gethostname(hostname, sizeof(hostname));

  /* Set up a TCP keepalive connection to the godot server */
  /* TODO: make the host and port more easily configurable */
  struct sockaddr_in addr = uv_ip4_addr(opts.host, opts.port);
  uv_tcp_init(loop, &client);
  uv_tcp_keepalive(&client, 1, 180);
  uv_tcp_connect(&connect_req, &client, addr, on_connect);

  uv_run(loop, UV_RUN_DEFAULT);
  return 0;
}
