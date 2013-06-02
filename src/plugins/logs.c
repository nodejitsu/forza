#include <stdlib.h>
#include <uv.h>
#include <estragon.h>

static uv_buf_t on_alloc(uv_handle_t* handle, size_t suggested_size) {
  return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void on_read(uv_stream_t* tcp, ssize_t nread, uv_buf_t rdbuf, const char* type) {
  char* str;
  estragon_metric_t* metric;

  if (nread == -1) {
    //
    // EOF. We should probably notify logging server, but ignore for now.
    //
    return;
  }

  metric = estragon_new_metric();

  str = malloc((nread + 1) * sizeof(char));
  memcpy(str, rdbuf.base, nread);
  str[nread] = '\0';

  metric->metric = 1.0;
  metric->description = str;
  metric->service = (type == "stdout") ? "logs/stdout" : "logs/stderr";

  estragon_send(metric);

  estragon_free_metric(metric);
  free(str);
  free(rdbuf.base);
}

void on_stdout_read(uv_stream_t* tcp, ssize_t nread, uv_buf_t rdbuf) {
  on_read(tcp, nread, rdbuf, "stdout");
}

void on_stderr_read(uv_stream_t* tcp, ssize_t nread, uv_buf_t rdbuf) {
  on_read(tcp, nread, rdbuf, "stderr");
}

void process_spawned_cb(uv_process_t* process, uv_process_options_t* options) {
  uv_read_start(options->stdio[1].data.stream, on_alloc, on_stdout_read);
  uv_read_start(options->stdio[2].data.stream, on_alloc, on_stderr_read);
}

int logs_init(estragon_plugin_t* plugin) {
  plugin->process_spawned_cb = process_spawned_cb;

  return 0;
}
