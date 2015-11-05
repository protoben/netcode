#include "util.h"

void die(int err, const char *fmt, ...) {
  char buf[1024];
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  if(err) {
    errno = err;
    perror(buf);
  } else {
    fprintf(stderr, "%s\n", buf);
  }

  exit(-1);
}

void *malloc_or_die(size_t s) {
  void *res = malloc(s);

  if(!res)
    die(ENOMEM, "malloc(%lu)");

  return res;
}

void *realloc_or_die(void *p, size_t s) {
  void *res = realloc(p, s);

  if(!res)
    die(ENOMEM, "realloc(%lu)");

  return res;
}

void *memdup_or_die(void *src, size_t s) {
  void *res = malloc_or_die(s);
  memcpy(res, src, s);
  return res;
}
