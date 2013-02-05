#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "options.h"

options_t options_parse(int argc, char *argv[]) {
  assert(argc > 1);

  options_t opts;

  opts.host = NULL;
  opts.port = 0;
  opts.pid = 0;
  opts.interval = 0;

  int i;
  for (i = 1; i < argc; i++) {
    switch((int)argv[i][0]) {
      case '-':
        switch((int)argv[i][1]) {
          case 'h':
            if (argv[i + 1][0] != '-') {
              opts.host = &argv[i + 1][0];
            }
            break;
          case 'p':
            if (argv[i + 1][0] != '-') {
              opts.port = atoi(&argv[i + 1][0]);
            }
            break;
          case 'f':
            if (argv[i + 1][0] != '-') {
              opts.pid = atoi(&argv[i + 1][0]);
            }
            break;
          case 'i':
            if (argv[i + 1][0] != '-') {
              opts.interval = atoi(&argv[i + 1][0]);
            }
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }
  return opts;
}

