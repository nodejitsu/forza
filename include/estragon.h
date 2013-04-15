#ifndef _ESTRAGON_H
#define _ESTRAGON_H

#include <uv.h>

typedef void (*estragon_process_exit)(int exit_status, int term_signal);
typedef void (*estragon_process_options)(uv_process_options_t);

struct estragon_plugin {
  estragon_process_exit process_exit_cb;
  estragon_process_options process_options_cb;
} typedef estragon_plugin_t;

#endif
