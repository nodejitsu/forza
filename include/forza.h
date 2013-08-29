#ifndef _FORZA_H
#define _FORZA_H

#include <uv.h>
#include <saneopt.h>

typedef void (*forza_process_exit_cb)(int exit_status, int term_signal);
typedef void (*forza_process_options_cb)(uv_process_options_t* options);
typedef void (*forza_process_spawned_cb)(uv_process_t* process, uv_process_options_t* options);
typedef void (*forza_stdio_data_cb)(char* data);
typedef void (*forza_connect_cb)(int status);

struct forza_plugin {
  forza_connect_cb connect_cb;
  forza_process_exit_cb process_exit_cb;
  forza_process_options_cb process_options_cb;
  forza_process_spawned_cb process_spawned_cb;
  forza_stdio_data_cb stdout_data_cb;
  forza_stdio_data_cb stderr_data_cb;
  forza_stdio_data_cb ipc_data_cb;
  saneopt_t* saneopt;
} typedef forza_plugin_t;

struct forza_metric_meta_app {
  char* name;
  char* user;
} typedef forza_metric_meta_app_t;

struct forza_metric_meta {
  int pid;
  long long uptime;
  unsigned short port;
  forza_metric_meta_app_t* app;
} typedef forza_metric_meta_t;

struct forza_metric {
  double metric;
  int ttl;
  time_t time;
  char* service;
  char* host;
  char* description;
  forza_metric_meta_t* meta;
} typedef forza_metric_t;

enum forza__stdio_type {
  STDIO_STDOUT,
  STDIO_STDERR,
  STDIO_IPC
} typedef forza__stdio_type_t;


void forza_connect(char** hosts, char* hostname, char* user, char* name, forza_connect_cb connect_cb_);
void forza_send(forza_metric_t* metric);
void forza_close();
forza_metric_t* forza_new_metric();
void forza_free_metric(forza_metric_t* metric);
char* forza_json_stringify(forza_metric_t* metric);

#endif
