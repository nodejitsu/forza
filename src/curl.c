#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <uv.h>
#include <curl/curlbuild.h>
#include <curl/curl.h>
#include "uv_queue.c"
#include "curl.h"

//
// Lesson: curl is terrible
//

void flush_queue(uv_async_t* handle, int status) {
  uv_close((uv_handle_t*)handle, (void (*)(uv_handle_t*))free);
  uv_curl_t* curl = handle->data;
  curl->waiting_on_flush = 0;
  curl_multi_perform( curl->multi_handle, &(curl->count) );
}

void notify_for_flush(uv_curl_t* curl) {
  uv_async_t* doflush = malloc(sizeof(uv_async_t));
  uv_async_init(curl->loop, doflush, flush_queue);
  doflush->data = curl;
  uv_async_send(doflush);
}

size_t read_from_queue(char *ptr, size_t size, size_t nmemb, void *userdata) {
  uv_curl_t* curl = (uv_curl_t*) userdata;
  uv_queue_t* queue = (uv_queue_t*) curl->queue;
  int to_write = size * nmemb;
  int offset = 0;
  while (to_write - offset > 0) {
    if (!queue->length) {
      break;
    }
    uv_buf_t head = queue->buffers[0];
    if (!head.len) {
      uv_queue_shift(queue, NULL);
      free(head.base);
    }
    else {
      int remaining_in_buf = head.len - to_write;
      int writing;
      if (remaining_in_buf >= 0) {
        writing = to_write - offset;
        memcpy(ptr + offset, head.base, writing);
        uv_buf_t tmp = queue->buffers[0] = uv_buf_init(malloc(remaining_in_buf), remaining_in_buf);
        memcpy(tmp.base, head.base + writing, remaining_in_buf);
        free(head.base);
      }
      else{
        writing = head.len;
        memcpy(ptr + offset, head.base, writing);
        uv_queue_shift(queue, NULL);
        free(head.base);
      }
      offset += writing;
    }
  }
  if (queue->length) {
    notify_for_flush(curl);
  }
  return offset;
}

void read_to_curl(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
  if (nread <= 0) {
    return;
  }
  const uv_buf_t cp = uv_buf_init(buf.base, nread);
  uv_curl_t* curl = (uv_curl_t*)stream->data;
  uv_queue_t* queue = curl->queue;
  uv_queue_push(queue, cp);
  if(!curl->waiting_on_flush) {
    curl->waiting_on_flush = 1;
    notify_for_flush(curl);
  }
}

uv_buf_t alloc_buffer(uv_handle_t* stream, size_t suggested_size) {
  return uv_buf_init(malloc(suggested_size), suggested_size);
}

int uv_curl_init(uv_loop_t* loop, uv_curl_t* curl, uv_pipe_t* pipe, char* addr) {
  if (curl_global_init(CURL_GLOBAL_ALL)) return 1;
  
  uv_queue_t* queue = (uv_queue_t*) malloc(sizeof(uv_queue_t));
  if(!queue) return 1;
  if(uv_queue_init(queue)) {
    free(queue);
    return 1;
  }
  
  curl->queue = queue;
  pipe->data = curl;
  
  CURL* easy_handle = curl->easy_handle = curl_easy_init();
  curl_easy_setopt(easy_handle, CURLOPT_URL, addr);
  curl_easy_setopt(easy_handle, CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(easy_handle, CURLOPT_READFUNCTION, read_from_queue);
  curl_easy_setopt(easy_handle, CURLOPT_READDATA, curl);
  
  CURLM* multi_handle = curl->multi_handle = curl_multi_init();
  curl_multi_add_handle(multi_handle, easy_handle);
  
  uv_read_start((uv_stream_t*)pipe, alloc_buffer, read_to_curl);

  return 0;
}

void uv_free_curl(uv_curl_t* curl) {
  curl_multi_remove_handle(curl->multi_handle, curl->easy_handle);
  curl_easy_cleanup(curl->easy_handle);
  curl_multi_cleanup(curl->multi_handle);
  uv_free_queue(curl->queue);
  free(curl);
}
