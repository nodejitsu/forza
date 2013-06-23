#ifndef _LOGS_H
#define _LOGS_H

#include <estragon.h>

enum logs__type {
  LOGS__STDOUT,
  LOGS__STDERR
} typedef logs__type_t;


int logs_init(estragon_plugin_t* plugin);

#endif
