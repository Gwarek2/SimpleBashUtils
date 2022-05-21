#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <locale.h>

#include "s21_cat.h"

int main(int argv, char *argc[]) {
    struct cat_state state;
    initialize_state(&state);
    setlocale(LC_ALL, "");
    if (argv > 1) {
        read_flags(&state, argv - 1, argc + 1);
        execute_cat(argc + 1, argv - 1, &state);
    } else {

    }

    return 0;
}

void initialize_state(struct cat_state *st) {
    st->flags.b_flag = false;
    st->flags.e_flag = false;
    st->flags.n_flag = false;
    st->flags.s_flag = false;
    st->flags.t_flag = false;
    st->line_count = 1;
    st->nl_previous = false;
}

size_t get_dash_index(const char *str) {
    return strspn(str, "-");
}

void read_flags(struct cat_state *st, size_t argv, char *args[]) {
    for (size_t i = 0; i < argv; i++)
        read_cat_flags(st, args[i]);
}

void read_cat_flags(struct cat_state *st, char *str) {
    size_t last_dash = get_dash_index(str);
    str += last_dash;
    if (last_dash == 2) {
        st->flags.b_flag |= !strcmp(str, "number-nonblank");
        st->flags.n_flag |= !strcmp(str, "number");
        st->flags.s_flag |= !strcmp(str, "squeeze-blank");
    } else if (last_dash == 1) {
        while (*str) {
            st->flags.b_flag |= *str == 'b';
            st->flags.e_flag |= *str == 'e' || *str == 'E';
            st->flags.n_flag |= *str == 'n';
            st->flags.s_flag |= *str == 's';
            st->flags.t_flag |= *str == 't' || *str == 'T';
            st->flags.v_flag |= *str == 'v' || *str == 'e' || *str == 't';
            str++;
        }
    }
}

void execute_cat(char *args[], size_t argv, struct cat_state *st) {
    for (size_t i = 0; i < argv; i++) {
        size_t last_dash = get_dash_index(args[i]);
        if (last_dash == 0 || last_dash > 2)
            print_file(args[i], st);
    }
}

void print_file(const char *filename, struct cat_state *st) {
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        while (print_line(f, st) != EOF);
        fclose(f);
    } else {
        print_error();
    }
}

int print_line(FILE *f_stream, struct cat_state *st) {
    int ch = getc(f_stream);

    bool only_newline = ch == '\n';
    st->nl_previous &= only_newline && st->flags.s_flag;
    if (st->flags.b_flag && !only_newline)
        printf(LINE_N_FMT, st->line_count++);
    else if (st->flags.n_flag && ch != EOF)
        printf(LINE_N_FMT, st->line_count++);

    while (ch != EOF && ch != '\n') {
        if (st->flags.t_flag && ch == '\t')
            puts(T_TAB);
        else
            putchar(ch);
        ch = getc(f_stream);
    }

    if (ch != EOF && !st->nl_previous) {
        if (st->flags.e_flag) putchar('$');
        putchar(ch);
    }

    st->nl_previous = only_newline && st->flags.s_flag;

    if (ferror(f_stream))
        print_error();

    return ch;
}

extern inline void print_error() {
    puts(strerror(errno));
}