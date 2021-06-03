#ifndef UTIL_HPP
#define UTIL_HPP

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef unsigned char uchar;
typedef u_int8_t byte;

// void print_in_hex(ustring s, int len);
void *global_malloc(size_t size);
void global_free(void *addr, size_t size);

#endif /* ifndef UTIL_HPP */
