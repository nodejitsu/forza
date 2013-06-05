#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <uv.h>

#include <estragon.h>

char hostname[512];
uv_tcp_t client;
uv_loop_t* loop;
static uv_connect_t connect_req;

int host_index = -1;
char** hosts;

void estragon__on_connect(uv_connect_t* req, int status);

void estragon__reconnect(estragon_connect_cb connect_cb) {
  char* pair;
  char* port_sep;
  char host[16];
  int port;
  struct sockaddr_in addr;

  ++host_index;
  pair = hosts[host_index];
  if (host == NULL) {
    host_index = 0;
    pair = hosts[host_index];
  }

  port_sep = strchr(pair, ':');
  strncpy(host, pair, port_sep - pair);
  host[port_sep - pair] = '\0';
  sscanf(port_sep + 1, "%d", &port);

  printf("connecting to %s:%d...\n", host, port);

  addr = uv_ip4_addr(host, port);

  loop = uv_default_loop();

  connect_req.data = (void*) connect_cb;

  /* Set up a TCP keepalive connection to the godot server */
  uv_tcp_init(loop, &client);
  uv_tcp_keepalive(&client, 1, 180);
  uv_tcp_connect(&connect_req, &client, addr, estragon__on_connect);
}

void estragon_connect(char** hosts_, estragon_connect_cb connect_cb) {
  /* Get the hostname so that it can be provided to the server */
  gethostname(hostname, sizeof(hostname));

  hosts = hosts_;

  estragon__reconnect(connect_cb);
}

void estragon__on_connect(uv_connect_t* req, int status) {
  if (status != 0) {
    fprintf(stderr, "connect error: %s\n", uv_strerror(uv_last_error(loop)));
    estragon__reconnect((estragon_connect_cb) req->data);
    return;
  }

  if (req->data != NULL) {
    ((estragon_connect_cb) req->data)(status);
  }
}

void estragon__on_write(uv_write_t* req, int status) {
  if (status) {
    fprintf(stderr, "write error: %s\n", uv_strerror(uv_last_error(loop)));
    estragon__reconnect(NULL);
  }
  free(req);
}

estragon_metric_t* estragon_new_metric() {
  estragon_metric_t* metric = malloc(sizeof(estragon_metric_t));
  if (metric == NULL) {
    return NULL;
  }

  metric->meta = malloc(sizeof(estragon_metric_meta_t));
  if (metric->meta == NULL) {
    free(metric);
    return NULL;
  }

  metric->service = NULL;
  metric->description = NULL;

  return metric;
}

void estragon_free_metric(estragon_metric_t* metric) {
  free(metric->meta);
  free(metric);
}

void estragon_send(estragon_metric_t* metric) {
  int r;
  uv_buf_t send_buf;
  uv_stream_t *stream;
  uv_write_t *write_req;
  char *json_data;

  metric->host = hostname;
  json_data = estragon_json_stringify(metric);

  write_req = malloc(sizeof *write_req);
  send_buf = uv_buf_init(json_data, sizeof(char) * strlen(json_data));
  stream = connect_req.handle;

  r = uv_write(write_req, stream, &send_buf, 1, estragon__on_write);
  if (r) {
    fprintf(stderr, "uv_write: %s\n", uv_strerror(uv_last_error(loop)));
    return;
  }
  free(json_data);
}

void estragon_close() {
  uv_close((uv_handle_t*) &client, NULL);
}
