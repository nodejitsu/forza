#include <uv.h>
#include "uv_queue.c"
#include <curl/curlbuild.h>
#include <curl/curl.h>

struct uv_curl_s {
  uv_loop_t* loop;
  uv_queue_t* queue;
  int waiting_on_flush;
  CURLM* multi_handle;
  CURL* easy_handle;
  int count;
  uv_pipe_t pipe;
};
typedef struct uv_curl_s uv_curl_t;

int uv_curl_init(uv_loop_t*, uv_curl_t*, uv_pipe_t*, char*);