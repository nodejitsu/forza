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
estragon_connect_cb connect_cb;

char* estragon__json_escape(char* string) {
  char* result = malloc((2 * strlen(string) + 1) * sizeof(char));
  result[0] = '\0';

  while (*string != '\0') {
    switch (*string) {
      case '\\': strcat(result, "\\\\"); break;
      case '"':  strcat(result, "\\\""); break;
      case '\b': strcat(result, "\\b");  break;
      case '\f': strcat(result, "\\f"); break;
      case '\n': strcat(result, "\\n"); break;
      case '\r': strcat(result, "\\r"); break;
      case '\t': strcat(result, "\\t"); break;
      default:   strncat(result, string, 1); break;
    }
    ++string;
  }

  return result;
}

void estragon__on_connect(uv_connect_t* req, int status) {
  connect_cb(status);
}

void estragon_connect(char* host, int port, estragon_connect_cb connect_cb_) {
  struct sockaddr_in addr = uv_ip4_addr(host, port);

  loop = uv_default_loop();

  /* Get the hostname so that it can be provided to the server */
  gethostname(hostname, sizeof(hostname));

  connect_cb = connect_cb_;
  /* Set up a TCP keepalive connection to the godot server */
  uv_tcp_init(loop, &client);
  uv_tcp_keepalive(&client, 1, 180);
  uv_tcp_connect(&connect_req, &client, addr, estragon__on_connect);
}

void estragon__json_append(char** string, const char* property, char* value, int prepend_comma) {
  // This is going to leak *so* bad.
  int property_length = strlen(property);
  int value_length = strlen(value);

  *string = realloc(*string, strlen(*string) + property_length + value_length + 4 + prepend_comma);

  if (prepend_comma) {
    strncat(*string, ",", 1);
  }
  strncat(*string, "\"", 1);
  strncat(*string, property, property_length);
  strncat(*string, "\":", 2);
  strncat(*string, value, value_length);
}

char* estragon__json_stringify_string(char* string) {
  char* escaped = estragon__json_escape(string);
  size_t length = strlen(escaped);
  char* out = malloc(length + 3);

  if (out == NULL) {
    return NULL;
  }

  out[0] = '\0';

  strncat(out, "\"", 1);
  strncat(out, escaped, length);
  strncat(out, "\"", 1);

  free(escaped);

  return out;
}

char* estragon__json_stringify(estragon_metric_t* metric) {
  char* json = malloc(1);
  char buf[128];
  char* str_buf;

  json[0] = '{';

  snprintf(buf, sizeof(buf), "%.8f", metric->metric);
  estragon__json_append(&json, "metric", buf, 0);

  if (metric->description) {
    str_buf = estragon__json_stringify_string(metric->description);
    estragon__json_append(&json, "description", str_buf, 1);
    free(str_buf);
  }

  if (metric->description) {
    str_buf = estragon__json_stringify_string(metric->service);
    estragon__json_append(&json, "service", str_buf, 1);
    free(str_buf);
  }

  json = realloc(json, strlen(json) + 3);
  strncat(json, "}\n", 2);

  return json;
}

void estragon__after_write(uv_write_t* req, int status) {
  if (status) {
    fprintf(stderr, "uv_write error: %s\n", uv_strerror(uv_last_error(loop)));
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
  char *json_data = estragon__json_stringify(metric);

  write_req = malloc(sizeof *write_req);
  send_buf = uv_buf_init(json_data, sizeof(char) * strlen(json_data));

  stream = connect_req.handle;

  r = uv_write(write_req, stream, &send_buf, 1, estragon__after_write);
  if (r) {
    fprintf(stderr, "uv_write: %s\n", uv_strerror(uv_last_error(loop)));
    return;
  }
  free(json_data);
}

void estragon_close() {
  uv_close((uv_handle_t*) &client, NULL);
}
