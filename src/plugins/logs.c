#include <uv.h>
#include <estragon.h>

static uv_buf_t on_alloc(uv_handle_t* handle, size_t suggested_size) {
  return uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void on_read(uv_stream_t* tcp, ssize_t nread, uv_buf_t rdbuf, const char* type) {
  if (nread == -1) {
    //
    // EOF. We should probably notify logging server, but ignore for now.
    //
    return;
  }

  char* str = malloc((nread + 1) * sizeof(char));
  memcpy(str, rdbuf.base, nread);
  str[nread] = '\0';
  estragon_send(type, type, str, 1.0);
  free(str);
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
