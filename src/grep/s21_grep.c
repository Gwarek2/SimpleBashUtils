#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

#include "s21_grep.h"
#include "../common/utils.h"

int main(int argc, char *argv[]) {
    struct grep_state state = GREP_DEFAULT;
    struct llist *patterns = initialize_llist();
    struct llist *files = initialize_llist();

    parse_cmd_args(argc - 1, argv + 1, &state, patterns, files);
    process_files(patterns->next, files->next, &state);

    free_llist(patterns);
    free_llist(files);
    return 0;
}

void parse_cmd_args(int argc, char *argv[], struct grep_state *st, struct llist *patterns, struct llist *files) {
    parse_regexes(argc, argv, patterns, st);
    parse_files(argc, argv, files, st);
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
            size_t s = 0;
            len = getline(&str, &s, f);
            if (len == -1) {
                free(str);
                break;
            }
            if (str[len - 1] == '\n') str[len - 1] = '\0';
            add_to_llist(patterns, str, true);
        }
        if (ferror(f))
            print_error();
    } else {
        print_error();
    }
}

void parse_files(int argc, char *argv[], struct llist *files, struct grep_state *st) {
    for (int i = 0; i < argc; i++) {
        bool is_opt = get_dash_index(argv[i]) == 1;
        if (is_opt && strpbrk(argv[i], "ef")) {
            i++;
        } else if (!is_opt) {
            add_to_llist(files, argv[i], false);
            st->files_to_search++;
        }
    }
}

void parse_options(int argc, char *argv[], struct grep_state *st) {
    for (int i = 0; i < argc; i++) {
        if (get_dash_index(argv[i]) == 1) {
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
                if (*opt_str == 'e' || *opt_str == 'f') i++;
                opt_str++;
            }
        }
    }
}

void process_files(struct llist *patterns, struct llist *files, struct grep_state *st) {
    while (files != NULL) {
        FILE *f = fopen(files->data, "r");
        if (f != NULL) {
            search_in_file(f, files->data, patterns, st);
            fclose(f);
        } else if (!st->flags.s) {
            print_error();
        }
        files = files->next;
    }
}

void search_in_file(FILE *f, char *filename, struct llist *patterns, struct grep_state *st) {
    ssize_t len = 0;
    size_t lines_count = 1;
    size_t match_count = 0;
    while (true) {
        char *buffer = NULL;
        size_t s = 0;
        len = getline(&buffer, &s, f);
        if (len == -1) {
            free(buffer);
            break;
        }
        bool match = find_patterns_in_line(buffer, patterns, st);
        if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        if (match && !st->flags.l && !st->flags.c) {
            output_line(buffer, filename, lines_count, st);
        }
        free(buffer);
        lines_count++;
        match_count += match;
    }
    output_filename_and_count(filename, match_count, st);
    if (ferror(f) && !st->flags.s)
        print_error();
}

bool find_patterns_in_line(char *line, struct llist *patterns, struct grep_state *st) {
    bool match = false;
    while (patterns != NULL && !match) {
        match = find_match(line, patterns->data, st);
        patterns = patterns->next;
    }
    return match;
}

bool find_match(char *line, char *pattern, struct grep_state *st) {
    bool match = false;
    int cflag = st->flags.i ? REG_ICASE : 0;
    regex_t re;
    int status = regcomp(&re, pattern, cflag | REG_NOSUB);
    if (status == 0) {
        match = !regexec(&re, line, 0, NULL, 0);
        regfree(&re);
        if (st->flags.v) match = !match;
    } else {
        print_regex_error(status, &re);
    }
    return match;
}

bool find_offsets(char *line, char *pattern, struct offset_array *pmatch_arr, struct grep_state *st) {
    bool match = false;
    int cflag = st->flags.i ? REG_ICASE : 0;
    regex_t re;

    int status = regcomp(&re, pattern, cflag);
    int i = 0;
    if (status == 0) {
        while (true) {
            int index = pmatch_arr->last_index;
            match = !regexec(&re, line + i, 1, pmatch_arr->data + index, 0);
            regmatch_t *new_array = realloc(pmatch_arr->data, sizeof(regmatch_t) * (index + 2));
            if (new_array == NULL)
                free(pmatch_arr->data);
            if (status != 0 || new_array == NULL) break;
            pmatch_arr->data = new_array;
            i = pmatch_arr->data[index].rm_eo;
            pmatch_arr->last_index++;
        }
    } else {
        print_regex_error(status, &re);
    }
    return match;
}

void output_line(char *line, char *filename, size_t line_number, struct grep_state *st) {
    if (!st->flags.h && st->files_to_search > 1)
        printf("%s:", filename);
    if (st->flags.n)
        printf("%zu:", line_number);
    puts(line);
}

void output_filename_and_count(char *filename, size_t match_count, struct grep_state *st) {
    if (st->flags.l) {
        puts(filename);
    } else if (st->flags.c) {
        if (!st->flags.h && st->files_to_search > 1) printf("%s:", filename);
        printf("%zu\n", match_count);
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

void free_llist(struct llist *ll) {
    while (ll != NULL) {
        struct llist *tmp = ll;
        ll = ll->next;
        if (tmp->heap_used)
            free(tmp->data);
        free(tmp);
    }
}

void print_regex_error(int err_code, regex_t *re) {
    char buff[128];
    regerror(err_code, re, buff, 128);
    printf("%s\n", buff);
}