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

    if (!status)
        status = parse_cmd_args(argc - 1, argv + 1, &state, patterns, files);
    if (!status && patterns->next != NULL) {
        if (files->next != NULL)
            status = process_files(patterns->next, files->next, &state);
        else
            status = process_stdio(patterns->next, &state);
    }

    free_llist(patterns);
    free_llist(files);
    if (status) print_error("grep", "");
    return status;
}

int parse_cmd_args(int argc, char *argv[], struct grep_state *st,
                   struct llist *patterns, struct llist *files) {
    int status = 0;
    status |= parse_regexes(argc, argv, patterns, st);
    status |= parse_filenames(argc, argv, files, st);
    parse_options(argc, argv, st);
    return status;
}

int parse_regexes(int argc, char *argv[], struct llist *patterns, struct grep_state *st) {
    // Search for patterns after -e and -f flags
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
    // Search for first pattern if no -e and -f flag were detected
    if (!st->flags.e && !st->flags.f) {
        bool match = false;
        int i = 0;
        while (!match && i < argc)
            match = get_dash_index(argv[i++]) != 1;
        if (match) {
            st->first_regex_index = i--;
            patterns = add_to_llist(patterns, argv[i], false);
        }
    }
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
        print_error("grep", filename);
    }
    return patterns;
}

int parse_filenames(int argc, char *argv[], struct llist *files, struct grep_state *st) {
    int start = st->first_regex_index;
    if (st->flags.e || st->flags.f) start = 0;
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
                st->flags.ext |= *opt_str == 'E';
                if (*opt_str == 'e' || *opt_str == 'f') i++;
                opt_str++;
            }
        }
    }
}

int process_stdio(struct llist *patterns, struct grep_state *st) {
    int status = 0;
    if (st->flags.v || st->flags.c || st->flags.l)
        status = search_matches_in_file(stdin, "(standard input)", patterns, st);
    else
        status = search_substrings_in_file(stdin, "(standard input)", patterns, st);
    return status;
}

int process_files(struct llist *patterns, struct llist *files, struct grep_state *st) {
    int status = 0;
    while (files != NULL) {
        FILE *f = fopen(files->data, "r");
        if (f != NULL) {
            if (st->flags.v || st->flags.c || st->flags.l)
                status = search_matches_in_file(f, files->data, patterns, st);
            else
                status = search_substrings_in_file(f, files->data, patterns, st);
            fclose(f);
        } else if (!st->flags.s) {
            print_error("grep", files->data);
        }
        files = files->next;
    }
    return status;
}

// Search only for match in file content and does not care about its offsets
int search_matches_in_file(FILE *f, char *filename, struct llist *patterns, struct grep_state *st) {
    int status = 0;
    size_t lines_count = 1;
    size_t match_count = 0;
    bool match = false;
    while (true) {
        char *buffer = NULL;
        ssize_t len = 0;
        size_t s = 0;
        if (!match || !st->flags.l) {
            len = getline(&buffer, &s, f);
            status = buffer == NULL;
        }
        if (len == -1 || (match && st->flags.l)) {
            if (buffer != NULL) free(buffer);
            break;
        }
        match = find_match_in_line(buffer, patterns, st);
        if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        if (match && !st->flags.l && !st->flags.c)
            output_line(buffer, filename, lines_count, st);
        free(buffer);
        lines_count++;
        match_count += match;
    }
    output_filename_and_count(filename, match_count, match, st);
    return status;
}

bool find_match_in_line(char *line, struct llist *patterns, struct grep_state *st) {
    bool match = false;
    while (patterns != NULL && (!match || st->flags.v)) {
        match |= find_match(line, patterns->data, st);
        patterns = patterns->next;
    }
    return match ^ st->flags.v;
}

bool find_match(char *line, char *pattern, struct grep_state *st) {
    bool match = false;
    int cflag = st->flags.i ? REG_ICASE : 0;
    cflag |= st->flags.ext ? REG_EXTENDED : 0;
    regex_t re;
    int status = regcomp(&re, pattern, cflag | REG_NOSUB);
    if (status == 0 && *pattern) {
        status = regexec(&re, line, 0, NULL, 0);
        regfree(&re);
        match = !status;
    } else if (*pattern) {
        print_regex_error(status, &re);
    } else {
        match = true;
    }
    return match;
}

// Search not only for match, but for its offsets too
int search_substrings_in_file(FILE *f, char *filename, struct llist *patterns, struct grep_state *st) {
    int status = 0;
    size_t lines_count = 1;
    while (true) {
        char *buffer = NULL;
        ssize_t len = 0;
        size_t s = 0;
        len = getline(&buffer, &s, f);
        struct offset_array pmatch_arr = { malloc(sizeof(regmatch_t)), 0 };
        if (len == -1 || pmatch_arr.data == NULL) {
            status = buffer == NULL || pmatch_arr.data == NULL;
            if (buffer != NULL) free(buffer);
            if (pmatch_arr.data != NULL) free(pmatch_arr.data);
            break;
        }
        if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        bool match = find_substrings_in_line(buffer, patterns, st, &pmatch_arr);
        if (pmatch_arr.data != NULL) {
            qsort(pmatch_arr.data, pmatch_arr.last_index, sizeof(regmatch_t), &regmatch_cmp);
            if (st->empty_pattern && !st->flags.o)
                output_line(buffer, filename, lines_count, st);
            else if (match)
                output_substrings(buffer, filename, lines_count, &pmatch_arr, st);
            free(pmatch_arr.data);
        }
        free(buffer);
        lines_count++;
    }
    return status;
}

bool find_substrings_in_line(char *line, struct llist *patterns,
                          struct grep_state *st, struct offset_array *pmatch_arr) {
    bool match = false;
    st->empty_pattern = false;
    while (patterns != NULL && pmatch_arr->data != NULL) {
        match |= find_substrings(line, patterns->data, pmatch_arr, st);
        patterns = patterns->next;
    }
    return match;
}

bool find_substrings(char *line, char *pattern, struct offset_array *pmatch_arr, struct grep_state *st) {
    bool match = false;
    int cflag = st->flags.i ? REG_ICASE : 0;
    cflag |= st->flags.ext ? REG_EXTENDED : 0;
    regex_t re;

    int status = regcomp(&re, pattern, cflag);
    if (status == 0 && *pattern) {
        int i = 0;
        while (true) {
            int index = pmatch_arr->last_index;
            status = regexec(&re, line + i, 1, pmatch_arr->data + index, 0) || !line[i];
            match |= !status;
            regmatch_t *new_array = realloc(pmatch_arr->data, sizeof(regmatch_t) * (index + 2));
            if (new_array != NULL)
                pmatch_arr->data = new_array;
            else
                free(pmatch_arr->data);
            if (new_array == NULL || status != 0) break;
            if (pmatch_arr->data[index].rm_so == pmatch_arr->data[index].rm_eo && line[i]) {
                i++;
                continue;
            }
            pmatch_arr->data[index].rm_so += i;
            pmatch_arr->data[index].rm_eo += i;
            pmatch_arr->last_index++;
            i = pmatch_arr->data[index].rm_eo;
        }
        st->empty_pattern &= !match;
        regfree(&re);
    } else if (*pattern) {
        print_regex_error(status, &re);
    } else {
        st->empty_pattern = true;
    }
    return match;
}

// Outputs just line with no higlight
void output_line(char *line, char *filename, size_t line_number, struct grep_state *st) {
    print_line_credentials(filename, line_number, st);
    puts(line);
}

// Outputs matching line with highlited matching substrings (only substrings if -o given)
void output_substrings(char *line, char *filename, size_t line_number,
                       struct offset_array *pmatch_arr, struct grep_state *st) {
    if (!st->flags.o) print_line_credentials(filename, line_number, st);
    int end = 0;
    for (size_t i = 0; i < pmatch_arr->last_index; i++) {
        if (pmatch_arr->data[i].rm_eo > end && pmatch_arr->data[i].rm_so >= end) {
            if (st->flags.o) print_line_credentials(filename, line_number, st);
            while (end < pmatch_arr->data[i].rm_so && !st->flags.o)
                putchar(line[end++]);
            int start = pmatch_arr->data[i].rm_so;
            end = pmatch_arr->data[i].rm_eo;
            // MATCH_HIGHLIGHT
            for (int j = start; j < end; j++)
                putchar(line[j]);
            // RESET
            if (st->flags.o) putchar('\n');
        }
    }
    int len = strlen(line);
    while (end < len && !st->flags.o)
        putchar(line[end++]);
    if (line[end - 1] != '\n' && !st->flags.o)
        putchar('\n');
}

// Outputs filename and/or line number if corresponding flags and conditions are present
void print_line_credentials(char *filename, size_t line_number, struct grep_state *st) {
    if (!st->flags.h && st->files_to_search > 1)
        printf("%s:", filename);
    if (st->flags.n)
        printf("%zu:", line_number);
}

// Output for -l and -c flags
void output_filename_and_count(char *filename, size_t match_count, bool match,  struct grep_state *st) {
    if (match && st->flags.l) {
        puts(filename);
    } else if ((match || !st->flags.l) && st->flags.c) {
        if (!st->flags.h && st->files_to_search > 1) printf("%s:", filename);
        printf("%zu\n", match_count);
    }
}

// Comparator for standard qsort
int regmatch_cmp(const void *offset1, const void *offset2) {
    const regmatch_t *pmatch1 = offset1;
    const regmatch_t *pmatch2 = offset2;
    int result = -1;
    if (pmatch2->rm_so < pmatch1->rm_so)
        result = 1;
    else if (pmatch2->rm_so == pmatch1->rm_so && pmatch2->rm_eo > pmatch1->rm_eo)
        result = 1;
    return result;
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


// Returns the last element
struct llist* add_to_llist(struct llist *ll, void *value, bool heap_used) {
    struct llist *cursor = ll;
    while (cursor->next != NULL)
        cursor = cursor->next;
    struct llist *new = malloc(sizeof(struct llist));
    if (new != NULL) {
        new->data = value;
        new->heap_used = heap_used;
        new->next = NULL;
    }
    cursor->next = new;
    return cursor->next;
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
