#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "utils.h"

void print_error(char *program, char *context) {
    if (*program)
        fprintf(stderr, "%s: ", program);
    if (*context)
        fprintf(stderr, "%s: ", context);
    fprintf(stderr, "%s\n", strerror(errno));
}

size_t get_dash_index(const char *str) {
    return strspn(str, "-");
}
