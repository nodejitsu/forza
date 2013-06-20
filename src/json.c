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

  json = realloc(json, strlen(json) + 3);
  strncat(json, "}\n", 2);

  return json;
}

