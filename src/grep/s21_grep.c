#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

#include "s21_grep.h"
#include "../common/utils.h"

int main(int argc, char *argv[]) {
    struct grep_state state;
    struct llist *patterns = initialize_llist();
    struct llist *files = initialize_llist();
    
    initialize_state(&state);
    parse_cmd_args(argc - 1, argv + 1, &state, patterns, files);

    free_list(patterns);
    free_list(files);
    return 0;
}

void initialize_state(struct grep_state *st) {
    st->flags.e = false;
    st->flags.i = false;
    st->flags.v = false;
    st->flags.c = false;
    st->flags.l = false;
    st->flags.n = false;
    st->flags.h = false;
    st->flags.s = false;
    st->flags.f = false;
    st->flags.o = false;
    st->match_count = 0;
    st->files_to_search = 0;
}

void parse_cmd_args(int argc, char *argv[], struct grep_state *st, struct llist *patterns, struct llist *files) {
    parse_regexes(argc, argv, patterns, st);
    parse_files(argc, argv, files);
    parse_options(argc, argv, st);
}

void parse_regexes(int argc, char *argv[], struct llist *patterns, struct grep_state *st) {
    for (int i = 0; i < argc - 1; i++) {
        bool is_opt = get_dash_index(argv[i]) == 1;
        if (is_opt && strchr(argv[i], 'e')) {
            i++;
            st->flags.e = true;
            add_to_llist(patterns, argv[i], false);
        } else if (is_opt && strchr(argv[i], 'f')) {
            i++;
            st->flags.f = true;
            read_regex_from_file(argv[i], patterns);
        }
    }
    if (!st->flags.e && !st->flags.f)
        add_to_llist(patterns, argv, false);
}

void read_regex_from_file(char *filename, struct llist *patterns) {
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        ssize_t len = 0;
        while (true) {
            char *str = NULL;
            len = getline(&str, 0, f);
            if (len == -1) {
                free(str);
                break;
            }
            add_to_llist(patterns, str, true);
        }
        if (ferror(f))
            print_error();
    } else {
        print_error();
    }
}

void parse_files(int argc, char *argv[], struct llist *files) {
    for (int i = 0; i < argc; i++) {
        bool is_opt = get_dash_index(argv[i]) == 1;
        if (is_opt && strpbrk(argv[i], "ef"))
            i += 1;
        else if (!is_opt)
            add_to_llist(files, argv[i], false);
    }
}

void parse_options(int argc, char *argv[], struct grep_state *st) {
    bool ef_flag_prev = false;
    for (int i = 0; i < argc; i++) {
        if (get_dash_index(argv[i]) == 1 && !ef_flag_prev) {
            char *opt_str = argv[i];
            while (*opt_str) {
                st->flags.i |= *opt_str == 'i';
                st->flags.v |= *opt_str == 'v';
                st->flags.c |= *opt_str == 'c';
                st->flags.l |= *opt_str == 'l';
                st->flags.n |= *opt_str == 'n';
                st->flags.h |= *opt_str == 'h';
                st->flags.s |= *opt_str == 's';
                st->flags.o |= *opt_str == 'o';
                ef_flag_prev = *opt_str == 'e' || *opt_str == 'f';
                opt_str++;
            }
        }
    }
}

struct llist* initialize_llist() {
    struct llist *ll = malloc(sizeof(struct llist));
    ll->next = NULL;
    ll->data = NULL;
    ll->heap_used = false;
    return ll;
}

void add_to_llist(struct llist *ll, void *value, bool heap_used) {
    struct llist *cursor = ll;
    while (cursor->next != NULL)
        cursor = cursor->next;
    struct llist *new = malloc(sizeof(struct llist));
    new->data = value;
    new->heap_used = heap_used;
    new->next = NULL;
    cursor->next = new;
}

void free_list(struct llist *ll) {
    while (ll != NULL) {
        struct llist *tmp = ll;
        ll = ll->next;
        if (tmp->heap_used)
            free(tmp->data);
        free(tmp);
    }
}
