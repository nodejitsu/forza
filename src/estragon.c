#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../deps/libuv/include/uv.h"

#include <saneopt.h>

#include <estragon.h>
#include <estragon-private/plugins.h>

static uv_loop_t *loop;
static uv_connect_t connect_req;
static uv_tcp_t client;
static char hostname[512];

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


void on_connect(uv_connect_t *req, int status) {
  if (status) {
    fprintf(stderr, "on_connect: %s\n", uv_strerror(uv_last_error(loop)));
    return;
  }
#ifdef DEBUG
  printf("Successfully connected!\n");
#endif

  /* Setup timers for heartbeat and resource reporting */
}

int main(int argc, char *argv[]) {
  printf("estragon "ESTRAGON_VERSION_HASH"\n");

  saneopt_t* opt = saneopt_init(argc, argv);
  int port = atoi(saneopt_get(opt, "port"));
  char* host = saneopt_get(opt, "host");

  loop = uv_default_loop();
  /* Get the hostname so that it can be provided to the server */
  gethostname(hostname, sizeof(hostname));

  /* Set up a TCP keepalive connection to the godot server */
  /* TODO: make the host and port more easily configurable */
  struct sockaddr_in addr = uv_ip4_addr(host, port);
  uv_tcp_init(loop, &client);
  uv_tcp_keepalive(&client, 1, 180);
  uv_tcp_connect(&connect_req, &client, addr, on_connect);

  uv_run(loop, UV_RUN_DEFAULT);
  return 0;
}
