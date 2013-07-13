#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <estragon.h>

char* estragon__json_escape(char* string) {
  char* result = malloc((2 * strlen(string) + 1) * sizeof(char));

  if (result == NULL) {
    return NULL;
  }

  result[0] = '\0';

  while (*string != '\0') {
    switch (*string) {
      case '\\': strcat(result, "\\\\"); break;
      case '"':  strcat(result, "\\\""); break;
      case '\b': strcat(result, "\\b");  break;
      case '\f': strcat(result, "\\f"); break;
      case '\n': strcat(result, "\\n"); break;
      case '\r': strcat(result, "\\r"); break;
      case '\t': strcat(result, "\\t"); break;
      default:   strncat(result, string, 1); break;
    }
    ++string;
  }

  return result;
}

void estragon__json_append(char** string, const char* property, char* value, int prepend_comma) {
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

char* estragon__json_stringify_string(char* string) {
  char* escaped = estragon__json_escape(string);
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

char* estragon__json_stringify_meta_app(estragon_metric_meta_app_t* app) {
  char* json = malloc(2);
  char* buf;

  json[0] = '{';
  json[1] = '\0';

  buf = estragon__json_stringify_string(app->user);
  estragon__json_append(&json, "user", buf, 0);
  free(buf);

  buf = estragon__json_stringify_string(app->name);
  estragon__json_append(&json, "name", buf, 1);
  free(buf);

  json = realloc(json, strlen(json) + 2);
  strncat(json, "}", 1);

  return json;
}

char* estragon__json_stringify_meta(estragon_metric_meta_t* meta) {
  char* json = malloc(2);
  char* app;
  char buf[128];
  int first = 0;

  json[0] = '{';
  json[1] = '\0';

  if (meta->uptime != ((long long int) - 1)) {
    snprintf(buf, sizeof(buf), "%lld", meta->uptime);
    estragon__json_append(&json, "uptime", buf, first++);
  }

  if (meta->port != ((unsigned short) - 1)) {
    snprintf(buf, sizeof(buf), "%hu", meta->port);
    estragon__json_append(&json, "port", buf, first++);
  }

  if (meta->app->user != NULL && meta->app->name != NULL) {
    app = estragon__json_stringify_meta_app(meta->app);
    estragon__json_append(&json, "app", app, first++);
    free(app);
  }

  json = realloc(json, strlen(json) + 2);
  strncat(json, "}", 1);

  return json;
}

char* estragon_json_stringify(estragon_metric_t* metric) {
  char* json = malloc(2);
  char buf[128];
  char* str_buf;


  json[0] = '{';
  json[1] = '\0';

  snprintf(buf, sizeof(buf), "%.8f", metric->metric);
  estragon__json_append(&json, "metric", buf, 0);

  snprintf(buf, sizeof(buf), "%lu", metric->time);
  estragon__json_append(&json, "time", buf, 1);

  str_buf = estragon__json_stringify_string(metric->host);
  estragon__json_append(&json, "host", str_buf, 1);
  free(str_buf);

  if (metric->description) {
    str_buf = estragon__json_stringify_string(metric->description);
    estragon__json_append(&json, "description", str_buf, 1);
    free(str_buf);
  }

  if (metric->service) {
    str_buf = estragon__json_stringify_string(metric->service);
    estragon__json_append(&json, "service", str_buf, 1);
    free(str_buf);
  }

  str_buf = estragon__json_stringify_meta(metric->meta);
  estragon__json_append(&json, "meta", str_buf, 1);
  free(str_buf);

  json = realloc(json, strlen(json) + 3);
  strncat(json, "}\n", 2);

  return json;
}

