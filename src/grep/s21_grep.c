#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

#include "s21_grep.h"
#include "../common/utils.h"

int main(int argc, char *argv[]) {
    int status = 0;
    struct grep_state state = GREP_DEFAULT;
    struct llist *patterns = initialize_llist();
    struct llist *files = initialize_llist();
    status = files == NULL || patterns == NULL;

    if (!status) {
        status = parse_cmd_args(argc - 1, argv + 1, &state, patterns, files) ||
                 process_files(patterns->next, files->next, &state);
    }

    free_llist(patterns);
    free_llist(files);
    if (status) print_error();
    return status;
}

int parse_cmd_args(int argc, char *argv[], struct grep_state *st, struct llist *patterns, struct llist *files) {
    int status = 0;
    status |= parse_regexes(argc, argv, patterns, st);
    status |= parse_files(argc, argv, files, st);
    parse_options(argc, argv, st);
    return status;
}

int parse_regexes(int argc, char *argv[], struct llist *patterns, struct grep_state *st) {
    for (int i = 0; i < argc - 1 && patterns != NULL; i++) {
        bool is_opt = get_dash_index(argv[i]) == 1;
        if (is_opt && strchr(argv[i], 'e')) {
            i++;
            st->flags.e = true;
            patterns = add_to_llist(patterns, argv[i], false);
        } else if (is_opt && strchr(argv[i], 'f')) {
            i++;
            st->flags.f = true;
            patterns = read_regex_from_file(argv[i], patterns);
        }
    }
    if (!st->flags.e && !st->flags.f)
        patterns = add_to_llist(patterns, *argv, false);
    return patterns == NULL;
}

struct llist* read_regex_from_file(char *filename, struct llist *patterns) {
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        while (true) {
            char *str = NULL;
            size_t s = 0;
            ssize_t len = 0;
            len = getline(&str, &s, f);
            if (len == -1 || patterns == NULL) {
                if (str != NULL) free(str);
                break;
            }
            if (str[len - 1] == '\n') str[len - 1] = '\0';
            patterns = add_to_llist(patterns, str, true);
        }
    } else {
        print_error();
    }
    return patterns;
}

int parse_files(int argc, char *argv[], struct llist *files, struct grep_state *st) {
    int start = 0;
    if (!st->flags.e && !st->flags.f) start = 1;
    for (int i = start; i < argc && files != NULL; i++) {
        bool is_opt = get_dash_index(argv[i]) == 1;
        if (is_opt && strpbrk(argv[i], "ef")) {
            i++;
        } else if (!is_opt) {
            files = add_to_llist(files, argv[i], false);
            st->files_to_search++;
        }
    }
    return files == NULL;
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

int process_files(struct llist *patterns, struct llist *files, struct grep_state *st) {
    int status = 0;
    while (files != NULL) {
        FILE *f = fopen(files->data, "r");
        if (f != NULL) {
            if (st->flags.v || st->flags.c || st->flags.l)
                status = search_in_file(f, files->data, patterns, st);
            else
                status = search_offsets_in_file(f, files->data, patterns, st);
            fclose(f);
        } else if (!st->flags.s) {
            print_error();
        }
        files = files->next;
    }
    return status;
}

int search_in_file(FILE *f, char *filename, struct llist *patterns, struct grep_state *st) {
    int status = 0;
    size_t lines_count = 1;
    size_t match_count = 0;
    while (true) {
        char *buffer = NULL;
        ssize_t len = 0;
        size_t s = 0;
        len = getline(&buffer, &s, f);
        if (len == -1) {
            status |= buffer == NULL;
            if (!status) free(buffer);
            break;
        }
        bool match = find_patterns_in_line(buffer, patterns, st);
        if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        if (match && !st->flags.l && !st->flags.c)
            output_line(buffer, filename, lines_count, st);
        free(buffer);
        lines_count++;
        match_count += match;
    }
    output_filename_and_count(filename, match_count, st);
    return status;
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
        status = regexec(&re, line, 0, NULL, 0);
        regfree(&re);
        match = (!status) ^ (st->flags.v);
    }
    if (status != 0 && status != REG_NOMATCH)
        print_regex_error(status, &re);
    return match;
}

int search_offsets_in_file(FILE *f, char *filename, struct llist *patterns, struct grep_state *st) {
    int status = 0;
    size_t lines_count = 1;
    while (true) {
        char *buffer = NULL;
        ssize_t len = 0;
        size_t s = 0;
        len = getline(&buffer, &s, f);
        struct offset_array pmatch_arr = { malloc(sizeof(regmatch_t)), 0 };
        if (len == -1) {
            status = buffer == NULL || pmatch_arr.data == NULL;
            if (buffer != NULL) free(buffer);
            if (pmatch_arr.data != NULL) free(pmatch_arr.data);
            break;
        }
        bool match = find_offsets_in_line(buffer, patterns, st, &pmatch_arr);
        if (pmatch_arr.data != NULL) {
            qsort(pmatch_arr.data, pmatch_arr.last_index, sizeof(regmatch_t), &regmatch_cmp);
            if (match) output_substrings(buffer, filename, lines_count, &pmatch_arr, st);
        }
        free(buffer);
        free(pmatch_arr.data);
        lines_count++;
    }
    return status;
}

bool find_offsets_in_line(char *line, struct llist *patterns, struct grep_state *st, struct offset_array *pmatch_arr) {
    bool match = false;
    while (patterns != NULL && pmatch_arr->data != NULL) {
        match |= find_offsets(line, patterns->data, pmatch_arr, st);
        patterns = patterns->next;
    }
    return match;
}

bool find_offsets(char *line, char *pattern, struct offset_array *pmatch_arr, struct grep_state *st) {
    bool match = false;
    int cflag = st->flags.i ? REG_ICASE : 0;
    regex_t re;

    int status = regcomp(&re, pattern, cflag);
    if (status == 0) {
        int i = 0;
        while (true) {
            int index = pmatch_arr->last_index;
            status = regexec(&re, line + i, 1, pmatch_arr->data + index, 0);
            match |= !status;
            regmatch_t *new_array = realloc(pmatch_arr->data, sizeof(regmatch_t) * (index + 2));
            if (new_array != NULL)
                pmatch_arr->data = new_array;
            else
                free(pmatch_arr->data);
            if (new_array == NULL || status != 0) break;
            pmatch_arr->data[index].rm_so += i;
            pmatch_arr->data[index].rm_eo += i;
            pmatch_arr->last_index++;
            i = pmatch_arr->data[index].rm_eo;
        }
        regfree(&re);
    } else {
        print_regex_error(status, &re);
    }
    return match;
}

void output_line(char *line, char *filename, size_t line_number, struct grep_state *st) {
    print_line_credentials(filename, line_number, st);
    puts(line);
}

void output_substrings(char *line, char *filename, size_t line_number, struct offset_array *pmatch_arr, struct grep_state *st) {
    if (!st->flags.o) print_line_credentials(filename, line_number, st);
    int end = 0;
    for (size_t i = 0; i < pmatch_arr->last_index; i++) {
        if (pmatch_arr->data[i].rm_eo > end && pmatch_arr->data[i].rm_so >= end) {
            if (st->flags.o) print_line_credentials(filename, line_number, st);
            while (end < pmatch_arr->data[i].rm_so && !st->flags.o)
                putchar(line[end++]);
            int start = pmatch_arr->data[i].rm_so;
            end = pmatch_arr->data[i].rm_eo;
            for (int j = start; j < end; j++) {
                RED putchar(line[j]); RED_END
            }
        }
    }
    int len = strlen(line);
    while (end < len)
        putchar(line[end++]);
}

void print_line_credentials(char *filename, size_t line_number, struct grep_state *st) {
    if (!st->flags.h && st->files_to_search > 1)
        printf("%s:", filename);
    if (st->flags.n)
        printf("%zu:", line_number);
}

void output_filename_and_count(char *filename, size_t match_count, struct grep_state *st) {
    if (st->flags.l) {
        puts(filename);
    } else if (st->flags.c) {
        if (!st->flags.h && st->files_to_search > 1) printf("%s:", filename);
        printf("%zu\n", match_count);
    }
}

int regmatch_cmp(const void *offset1, const void *offset2) {
    const regmatch_t *pmatch1 = offset1;
    const regmatch_t *pmatch2 = offset2;
    int bigger = 0;
    if (pmatch2->rm_so < pmatch1->rm_so)
        bigger = 1;
    else if (pmatch2->rm_so == pmatch1->rm_so && pmatch2->rm_eo < pmatch1->rm_eo)
        bigger = 1;
    return bigger;
}

struct llist* initialize_llist() {
    struct llist *ll = malloc(sizeof(struct llist));
    if (ll != NULL) {
        ll->next = NULL;
        ll->data = NULL;
        ll->heap_used = false;
    }
    return ll;
}

struct llist* add_to_llist(struct llist *ll, void *value, bool heap_used) {
    struct llist *cursor = ll;
    while (cursor->next != NULL)
        cursor = cursor->next;
    struct llist *new = malloc(sizeof(struct llist));
    if (new != NULL) {
        new->data = value;
        new->heap_used = heap_used;
        new->next = NULL;
        cursor->next = new;
    }
    return cursor;
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
    fputs(buff, stderr);
    putchar('\n');
}
