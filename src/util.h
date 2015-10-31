#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#ifndef UTIL_H
#define UTIL_H

void die(int err, const char *fmt, ...);
void *malloc_or_die(size_t s);
void *realloc_or_die(void *p, size_t s);
void *memdup_or_die(void *src, size_t s);

#endif
