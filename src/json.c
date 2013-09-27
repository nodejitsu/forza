#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <forza.h>

char* forza__json_escape(char* string) {
  char* result;
  int i;
  int length = 0;
  int str_len = strlen(string);

  for (i = 0; i < str_len; i++) {
    switch (string[i]) {
      case '\\':
      case '"':
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t':   length += 2; break;
      case '\x1b': length += 6; break;
      default:     length++;
    }
  }

  result = malloc(sizeof(*result) * (length + 1));

  if (result == NULL) {
    return NULL;
  }

  result[0] = '\0';

  for (i = 0; i < str_len; i++) {
    switch (string[i]) {
      case '\\':   strcat(result, "\\\\"); break;
      case '"':    strcat(result, "\\\""); break;
      case '\b':   strcat(result, "\\b");  break;
      case '\f':   strcat(result, "\\f"); break;
      case '\n':   strcat(result, "\\n"); break;
      case '\r':   strcat(result, "\\r"); break;
      case '\t':   strcat(result, "\\t"); break;
      case '\x1b': strcat(result, "\\u001b"); break;
      default:     strncat(result, &string[i], 1); break;
    }
  }

  return result;
}

void forza__json_append(char** string, const char* property, char* value, int prepend_comma) {
  // This is going to leak *so* bad.
  int property_length = strlen(property);
  int value_length = strlen(value);

  *string = realloc(*string, strlen(*string) + property_length + value_length + 4 + prepend_comma);

  if (prepend_comma) {
    strncat(*string, ",", 1);
  }
  strncat(*string, "\"", 1);
  strncat(*string, property, property_length);
  strncat(*string, "\":", 2);
  strncat(*string, value, value_length);
}

char* forza__json_stringify_string(char* string) {
  char* escaped = forza__json_escape(string);
  char* out;
  size_t length;

  if (escaped == NULL) {
    return NULL;
  }

  length = strlen(escaped);
  out = malloc(length + 3);

  if (out == NULL) {
    free(escaped);
    return NULL;
  }

  out[0] = '\0';

  strncat(out, "\"", 1);
  strncat(out, escaped, length);
  strncat(out, "\"", 1);

  free(escaped);

  return out;
}

char* forza__json_stringify_meta_app(forza_metric_meta_app_t* app) {
  char* json = malloc(2);
  char* buf;

  json[0] = '{';
  json[1] = '\0';

  buf = forza__json_stringify_string(app->user);
  forza__json_append(&json, "user", buf, 0);
  free(buf);

  buf = forza__json_stringify_string(app->name);
  forza__json_append(&json, "name", buf, 1);
  free(buf);

  json = realloc(json, strlen(json) + 2);
  strncat(json, "}", 1);

  return json;
}

char* forza__json_stringify_meta(forza_metric_meta_t* meta) {
  char* json = malloc(2);
  char* app;
  char buf[128];
  int first = 0;

  json[0] = '{';
  json[1] = '\0';

  if (meta->uptime != ((long long int) - 1)) {
    snprintf(buf, sizeof(buf), "%lld", meta->uptime);
    forza__json_append(&json, "uptime", buf, first++);
  }

  if (meta->port != ((unsigned short) - 1)) {
    snprintf(buf, sizeof(buf), "%hu", meta->port);
    forza__json_append(&json, "port", buf, first++);
  }

  if (meta->app->user != NULL && meta->app->name != NULL) {
    app = forza__json_stringify_meta_app(meta->app);
    forza__json_append(&json, "app", app, first++);
    free(app);
  }

  json = realloc(json, strlen(json) + 2);
  strncat(json, "}", 1);

  return json;
}

char* forza_json_stringify(forza_metric_t* metric) {
  char* json = malloc(2);
  char buf[128];
  char* str_buf;


  json[0] = '{';
  json[1] = '\0';

  snprintf(buf, sizeof(buf), "%.8f", metric->metric);
  forza__json_append(&json, "metric", buf, 0);

  snprintf(buf, sizeof(buf), "%lu", metric->time);
  forza__json_append(&json, "time", buf, 1);

  str_buf = forza__json_stringify_string(metric->host);
  forza__json_append(&json, "host", str_buf, 1);
  free(str_buf);

  if (metric->description) {
    str_buf = forza__json_stringify_string(metric->description);
    forza__json_append(&json, "description", str_buf, 1);
    free(str_buf);
  }

  if (metric->service) {
    str_buf = forza__json_stringify_string(metric->service);
    forza__json_append(&json, "service", str_buf, 1);
    free(str_buf);
  }

  str_buf = forza__json_stringify_meta(metric->meta);
  forza__json_append(&json, "meta", str_buf, 1);
  free(str_buf);

  json = realloc(json, strlen(json) + 3);
  strncat(json, "}\n", 2);

  return json;
}

