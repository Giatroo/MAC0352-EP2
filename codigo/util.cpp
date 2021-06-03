#include "util.hpp"

#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>

// void print_in_hex(ustring s, int len) {
// fprintf(stdout, "'");
// for (int i = 0; i < len; ++i) fprintf(stdout, "%02x ", s[i]);
// fprintf(stdout, "'\n\n");
// }

void *global_malloc(size_t size) {
    void *allocated_bytes = mmap(NULL, size, PROT_READ | PROT_WRITE,
                                 MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    if (allocated_bytes == MAP_FAILED) {
        fprintf(stderr,
                "Failed to allocate the bytes with mmap.\nErrno = "
                "%d.\nExiting...\n",
                errno);
        exit(errno);
    }

    return allocated_bytes;
}

void global_free(void *addr, size_t size) {
    int err = munmap(addr, size);

    if (err != 0) {
        fprintf(stderr, "Um erro ocorreu ao tentar desalocar um mmap\n");
        exit(errno);
    }
}
