#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "utils.h"

void print_error() {
    fputs(strerror(errno), stderr);
    putchar('\n');
}

size_t get_dash_index(const char *str) {
    return strspn(str, "-");
}
