#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include "s21_cat.h"
#include "../common/utils.h"

int main(int argv, char *argc[]) {
    struct cat_state state = CAT_DEFAULT;
    setlocale(LC_ALL, "");

    read_flags(&state, argv - 1, argc + 1);
    if (state.filenames)
        execute_cat_files(argc + 1, argv - 1, &state);
    else
        execute_cat_stdin(&state);

    return 0;
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
    } else {
        st->filenames = true;
    }
    st->flags.n_flag &= !st->flags.b_flag;
}

void execute_cat_files(char *args[], size_t argv, struct cat_state *st) {
    for (size_t i = 0; i < argv; i++) {
        size_t last_dash = get_dash_index(args[i]);
        if (last_dash == 0 || last_dash > 2)
            print_file(args[i], st);
        #ifdef __APPLE__
        st->line_count = 1;
        #endif
    }
}

void execute_cat_stdin(struct cat_state *st) {
    while (true) print_line(stdin, st);
}

void print_file(const char *filename, struct cat_state *st) {
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        while (print_line(f, st) != EOF) {}
        fclose(f);
    } else {
        print_error();
    }
}

int print_line(FILE *f_stream, struct cat_state *st) {
    int ch = getc(f_stream);
    bool only_newline = ch == '\n';
    bool is_eof = ch == EOF;
    #ifdef __APPLE__
    bool is_prev_nl = true;
    #else
    bool is_prev_nl = st->last_symbol == '\n';
    #endif
    st->empty_prev_line &= only_newline && st->flags.s_flag;

    if (st->flags.b_flag && !only_newline && !is_eof && is_prev_nl)
        printf(LINE_N_FMT, st->line_count++);
    else if (st->flags.n_flag && !is_eof && !st->empty_prev_line && is_prev_nl)
        printf(LINE_N_FMT, st->line_count++);

    while (ch != EOF && ch != '\n') {
        if (st->flags.t_flag && ch == '\t')
            printf("%s", T_TAB);
        else if (st->flags.v_flag)
            print_v_format(ch);
        else
            putchar(ch);
        st->last_symbol = ch;
        ch = getc(f_stream);
    }

    if (ch != EOF) st->last_symbol = ch;
    if (ch != EOF && !st->empty_prev_line) {
        if (st->flags.e_flag) putchar('$');
        putchar(ch);
    }

    st->empty_prev_line = only_newline && st->flags.s_flag;

    if (ferror(f_stream))
        print_error();

    return ch;
}

void print_v_format(int ch) {
    int converted_ch = ch % 128;
    if (ch == 127)
        printf("^%c", converted_ch - 64);
    else if (ch >= 128 && converted_ch == 1)
        printf("%s^%c", M_NOT, converted_ch - 64);
    else if (ch < 128 &&
            (converted_ch < 32 || converted_ch == 127) &&
            converted_ch != 9 && converted_ch != 10)
        printf("^%c", converted_ch + 64);
    else if (ch >= 128 && converted_ch < 32)
        printf("%s^%c", M_NOT, converted_ch + 64);
    else if (ch >= 128)
        printf("%s%c", M_NOT, converted_ch);
    else
        printf("%c", converted_ch);
}
