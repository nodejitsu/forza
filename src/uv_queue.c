#ifndef UV_QUEUE
#define UV_QUEUE
//
// A terrible queue for buffers in libuv
//
#include <uv.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
struct uv_queue_s {
  int length;
  uv_mutex_t* mutex;
  uv_buf_t* buffers;
};
//
// index - where data changed
// change - if negative that is how many were remove AFTER index
//
typedef struct uv_queue_s uv_queue_t;

//
// QUEUE IS NOT THREAD SAFE UNTIL THIS RETURNS
// SERIOUSLY WHO DOES STUFF BEFORE INIT IS DONE
//
int uv_queue_init (uv_queue_t* queue) {
  queue->buffers = (uv_buf_t*) malloc(0);
  //printf(__FILE__":%d alloc %p\n",__LINE__,queue->buffers);
  if (!queue->buffers) {
    return -1;
  }
  queue->mutex = (uv_mutex_t*) malloc(sizeof(uv_mutex_t));
  if (!queue->mutex) {
    return -1;
  }
  queue->length = 0;
  if (uv_mutex_init(queue->mutex)) {
    return -1;
  }
  return 0;
}

int uv_queue_push(uv_queue_t* queue, uv_buf_t buffer) {
  while (uv_mutex_trylock(queue->mutex));
    uv_buf_t* buffers = (uv_buf_t*) realloc(queue->buffers, (queue->length + 1) * sizeof(uv_buf_t));
    //printf("size %d len %d sizeof buf %d\n",(queue->length + 1) * sizeof(uv_buf_t), queue->length, sizeof(uv_buf_t));
    //printf(__FILE__":%d alloc %p\n",__LINE__,buffers);
    if (!buffers) {
      return -1;
    }
    queue->buffers = buffers;
    queue->buffers[queue->length] = buffer;
    queue->length += 1;
  uv_mutex_unlock(queue->mutex);
  return 0;
}

int uv_queue_shift(uv_queue_t* queue, uv_buf_t* destination) {
  if (!queue->length) {
    return -1;
  }
  while (uv_mutex_trylock(queue->mutex));
    uv_buf_t* buffers = queue->buffers;
    if (destination) {
      destination->base = buffers[0].base;
      destination->len = buffers[0].len;
    }
    int new_length = queue->length - 1;
    int new_size = new_length * sizeof(uv_buf_t);
    memmove(buffers, buffers + 1, new_size);
    queue->buffers = realloc(buffers, new_size);
    queue->length = new_length;
  uv_mutex_unlock(queue->mutex);
  return 0;
}

void uv_free_queue (uv_queue_t* queue) {
  //
  // Wait on other threads
  //
  while (uv_mutex_trylock(queue->mutex));
    int i = 0;
    for (i = 0; i < queue->length; i++) {
      free(queue->buffers[i].base);
    }
    free(queue->buffers);
  uv_mutex_destroy(queue->mutex);
}
#endif