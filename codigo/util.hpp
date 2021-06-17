
#ifndef UTIL_HPP
#define UTIL_HPP

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <string>

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

#define debug(x)  std::cout << #x << " = " << x << std::endl

typedef unsigned char uchar;
typedef unsigned char *ustring;
typedef u_int8_t byte;

typedef enum {
   ADDUSER,
   PASSWD,
   LOGIN,
   LEADERS,
   LIST,
   BEGIN,
   SEND,
   DELAY,
   END,
   LOGOUT,
   YES,
   NO,
   EXIT
} cmd_t;

// Converte um command em seu enum equivalente
int cmd_str_to_int(std::string cmd);

void print_in_hex(ustring s, int len);
void *global_malloc(size_t size);
void global_free(void *addr, size_t size);

void set_str(char *istr, std::string ostr);

ustring int_to_2byte_str(int);
int byte_str_to_int(ustring recvline, int pos);
void write_string(ustring outstr, int &pos, std::string str_to_write);
void read_string(ustring instr, int &pos, std::string &str_to_write);

#endif /* ifndef UTIL_HPP */

