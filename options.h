#ifndef _options_h
#define _options_h

struct options_s {
  char *host;
  int port;
  int pid;
  int interval;
};

typedef struct options_s options_t;

options_t options_parse(int argc, char *argv[]);

#endif
