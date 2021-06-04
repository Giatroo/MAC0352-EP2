#ifndef UTIL_HPP
#define UTIL_HPP

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

typedef unsigned char uchar;
typedef unsigned char* ustring;
typedef u_int8_t byte;

// void print_in_hex(ustring s, int len);
void *global_malloc(size_t size);
void global_free(void *addr, size_t size);

#endif /* ifndef UTIL_HPP */
