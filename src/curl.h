#include <uv.h>
#include "uv_queue.c"
#include <curl/curlbuild.h>
#include <curl/curl.h>

//
// A connection from a uv_pipe_t to a CURL URL
//
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

//
// Attach a uv_curl_t to a loop and pipe,
// send all data recieved on the pipe to the CURL compatible URL
//
int uv_curl_init(uv_loop_t*, uv_curl_t*, uv_pipe_t* input_stream, char* curl_url);
//
// Free all resources associated with a uv_curl_t
//
void uv_free_curl(uv_curl_t*);