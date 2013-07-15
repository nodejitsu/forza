#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <uv.h>

#include <estragon.h>

static char* hostname;
static uv_tcp_t client;
static uv_loop_t* loop;
static uv_connect_t connect_req;

static int host_index = -1;
static char** hosts;

void estragon__on_connect(uv_connect_t* req, int status);
void estragon__reconnect_on_close(uv_handle_t* handle);
void estragon__reconnect_on_connect_error(uv_handle_t* handle);

void estragon__reconnect(estragon_connect_cb connect_cb) {
  char* pair;
  char* port_sep;
  char host[16];
  int r;
  int port;
  struct sockaddr_in addr;

  ++host_index;
  pair = hosts[host_index];
  if (pair == NULL) {
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
  r = uv_tcp_connect(&connect_req, &client, addr, estragon__on_connect);

  if (r != 0) {
    //
    // This could potentially be one of the following errors:
    //
    //   * UV_EINVAL - wrong address family (IPv6 address is passed as a host)
    //
    // Remark (mmalecki): anything else? We should support IPv6 addresses in
    // the future.
    //
    // In all of those cases we should try the next host and reconnect.
    //
    fprintf(stderr, "connect error: %s\n", uv_strerror(uv_last_error(loop)));
    estragon__reconnect((estragon_connect_cb) connect_req.data);
    return;
  }
}

void estragon_connect(char** hosts_, char* hostname_, estragon_connect_cb connect_cb) {
  /* Get the hostname so that it can be provided to the server */
  hostname = hostname_;
  hosts = hosts_;
  estragon__reconnect(connect_cb);
}

void estragon__on_connect(uv_connect_t* req, int status) {
  if (status != 0) {
    fprintf(stderr, "connect error: %s\n", uv_strerror(uv_last_error(loop)));
    uv_close((uv_handle_t*) &client, estragon__reconnect_on_connect_error);
    return;
  }

  printf("connected.\n");

  if (req->data != NULL) {
    ((estragon_connect_cb) req->data)(status);
  }
}

void estragon__reconnect_on_close(uv_handle_t* handle) {
  estragon__reconnect(NULL);
}

void estragon__reconnect_on_connect_error(uv_handle_t* handle) {
  estragon_connect_cb cb = (connect_req.data != NULL)
    ? ((estragon_connect_cb) connect_req.data)
    : NULL;
  estragon__reconnect(cb);
}

void estragon__on_write(uv_write_t* req, int status) {
  if (status) {
    fprintf(stderr, "write error: %s\n", uv_strerror(uv_last_error(loop)));

    if (!uv_is_closing((uv_handle_t*) &client)) {
      uv_close((uv_handle_t*) &client, estragon__reconnect_on_close);
    }
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

  metric->time = (time_t) - 1;
  metric->service = NULL;
  metric->description = NULL;

  metric->meta->uptime = (long long int) - 1;
  metric->meta->port = (unsigned short) - 1;

  metric->meta->app = malloc(sizeof(estragon_metric_meta_app_t));
  metric->meta->app->user = NULL;
  metric->meta->app->name = NULL;

  return metric;
}

void estragon_free_metric(estragon_metric_t* metric) {
  free(metric->meta->app);
  free(metric->meta);
  free(metric);
}

void estragon_send(estragon_metric_t* metric) {
  int r;
  uv_buf_t send_buf;
  uv_stream_t *stream;
  uv_write_t *write_req;
  char *json_data;

  if (!uv_is_writable(connect_req.handle)) {
    return;
  }

  metric->host = hostname;

  if (metric->time == ((time_t) - 1)) {
    metric->time = time(NULL);
  }

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
