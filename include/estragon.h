#ifndef _ESTRAGON_H
#define _ESTRAGON_H

#include <uv.h>

typedef void (*estragon_process_exit_cb)(int exit_status, int term_signal);
typedef void (*estragon_process_options_cb)(uv_process_options_t);
typedef void (*estragon_connect_cb)(int status);

struct estragon_plugin {
  estragon_connect_cb connect_cb;
  estragon_process_exit_cb process_exit_cb;
  estragon_process_options_cb process_options_cb;
} typedef estragon_plugin_t;

void estragon_connect(char* host, int port, estragon_connect_cb connect_cb_);
void estragon_send(char* name, char* state, double value);

#endif
