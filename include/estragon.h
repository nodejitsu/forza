#ifndef _ESTRAGON_H
#define _ESTRAGON_H

#include <uv.h>
#include <saneopt.h>

typedef void (*estragon_process_exit_cb)(int exit_status, int term_signal);
typedef void (*estragon_process_options_cb)(uv_process_options_t* options);
typedef void (*estragon_process_spawned_cb)(uv_process_t* process, uv_process_options_t* options);
typedef void (*estragon_stdio_data_cb)(char* data);
typedef void (*estragon_connect_cb)(int status);

struct estragon_plugin {
  estragon_connect_cb connect_cb;
  estragon_process_exit_cb process_exit_cb;
  estragon_process_options_cb process_options_cb;
  estragon_process_spawned_cb process_spawned_cb;
  estragon_stdio_data_cb stdout_data_cb;
  estragon_stdio_data_cb stderr_data_cb;
  estragon_stdio_data_cb ipc_data_cb;
  saneopt_t* saneopt;
} typedef estragon_plugin_t;

struct estragon_metric_meta_app {
  char* name;
  char* user;
} typedef estragon_metric_meta_app_t;

struct estragon_metric_meta {
  int pid;
  long long uptime;
  unsigned short port;
  estragon_metric_meta_app_t* app;
} typedef estragon_metric_meta_t;

struct estragon_metric {
  double metric;
  int ttl;
  time_t time;
  char* service;
  char* host;
  char* description;
  estragon_metric_meta_t* meta;
} typedef estragon_metric_t;

enum estragon__stdio_type {
  STDIO_STDOUT,
  STDIO_STDERR,
  STDIO_IPC
} typedef estragon__stdio_type_t;


void estragon_connect(char** hosts, char* hostname, estragon_connect_cb connect_cb_);
void estragon_send(estragon_metric_t* metric);
void estragon_close();
estragon_metric_t* estragon_new_metric();
void estragon_free_metric(estragon_metric_t* metric);
char* estragon_json_stringify(estragon_metric_t* metric);

#endif
