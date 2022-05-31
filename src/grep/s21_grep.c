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
    if (!state.fatal_error && patterns->next != NULL) {
        if (files->next != NULL)
            process_files(patterns->next, files->next, &state);
        else
            process_stdio(patterns->next, &state);
    }

    free_llist(patterns);
    free_llist(files);
    if (state.fatal_error) print_error("grep", "");
    return state.fatal_error ? EXIT_FAILURE : EXIT_SUCCESS;
}

void parse_cmd_args(int argc, char *argv[], struct grep_state *st,
                   struct llist *patterns, struct llist *files) {
    parse_regexes(argc, argv, patterns, st);
    parse_filenames(argc, argv, files, st);
    parse_options(argc, argv, st);
}

void parse_regexes(int argc, char *argv[], struct llist *patterns, struct grep_state *st) {
    // Search for patterns after -e and -f flags
    for (int i = 0; i < argc - 1 && patterns != NULL; i++) {
        bool is_opt = get_dash_index(argv[i]) == 1;
        if (is_opt && strchr(argv[i], 'e')) {
            i++;
            st->options.e = true;
            patterns = add_to_llist(patterns, argv[i], false);
        } else if (is_opt && strchr(argv[i], 'f')) {
            i++;
            st->options.f = true;
            patterns = read_regex_from_file(argv[i], patterns);
        }
    }
    // Search for first pattern if no -e and -f flag were detected
    if (!st->options.e && !st->options.f) {
        bool match = false;
        int i = 0;
        while (!match && i < argc)
            match = get_dash_index(argv[i++]) != 1;
        if (match && patterns != NULL) {
            st->first_regex_index = i--;
            patterns = add_to_llist(patterns, argv[i], false);
        }
    }
    st->fatal_error |= patterns == NULL;
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
                free(str);
                break;
            }
            if (str[len - 1] == '\n') str[len - 1] = '\0';
            patterns = add_to_llist(patterns, str, true);
        }
        fclose(f);
    } else {
        print_error("grep", filename);
    }
    return patterns;
}

void parse_filenames(int argc, char *argv[], struct llist *files, struct grep_state *st) {
    int start = st->first_regex_index;
    if (st->options.e || st->options.f) start = 0;
    for (int i = start; i < argc && files != NULL; i++) {
        bool is_opt = get_dash_index(argv[i]) == 1;
        if (is_opt && strpbrk(argv[i], "ef")) {
            i++;
        } else if (!is_opt) {
            files = add_to_llist(files, argv[i], false);
            st->files_to_search++;
        }
    }
    st->fatal_error |= files == NULL;
}

void parse_options(int argc, char *argv[], struct grep_state *st) {
    for (int i = 0; i < argc; i++) {
        if (get_dash_index(argv[i]) == 1) {
            char *opt_str = argv[i];
            while (*opt_str) {
                st->options.v |= *opt_str == 'v';
                st->options.c |= *opt_str == 'c';
                st->options.l |= *opt_str == 'l';
                st->options.n |= *opt_str == 'n';
                st->options.h |= *opt_str == 'h';
                st->options.s |= *opt_str == 's';
                st->options.o |= *opt_str == 'o';
                if (*opt_str == 'i') st->cflags |= REG_ICASE;
                if (*opt_str == 'E') st->cflags |= REG_EXTENDED;
                if (*opt_str == 'e' || *opt_str == 'f') i++;
                opt_str++;
            }
        }
    }
}

void process_stdio(struct llist *patterns, struct grep_state *st) {
    if (st->options.v || st->options.c || st->options.l)
        search_matches_in_file(stdin, "(standard input)", patterns, st);
    else
         search_substrings_in_file(stdin, "(standard input)", patterns, st);
}

void process_files(struct llist *patterns, struct llist *files, struct grep_state *st) {
    while (files != NULL && !st->regex_error) {
        FILE *f = fopen(files->data, "r");
        if (f != NULL) {
            if (st->options.v || st->options.c || st->options.l)
                search_matches_in_file(f, files->data, patterns, st);
            else
                search_substrings_in_file(f, files->data, patterns, st);
            fclose(f);
        } else if (!st->options.s) {
            print_error("grep", files->data);
        }
        files = files->next;
    }
}

// Searches only for match in file content and does not care about its offsets
void search_matches_in_file(FILE *f, char *filename, struct llist *patterns, struct grep_state *st) {
    size_t lines_count = 1;
    size_t match_count = 0;
    bool match = false;
    while (true) {
        char *buffer = NULL;
        ssize_t len = 0;
        size_t s = 0;
        if ((!match || !st->options.l) && !st->regex_error) {
            len = getline(&buffer, &s, f);
            st->fatal_error = buffer == NULL;
        }
        if (len == -1 || (match && st->options.l) || st->regex_error || st->fatal_error) {
            free(buffer);
            break;
        }
        match = find_match_in_line(buffer, patterns, st);
        if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        if (match && !st->options.l && !st->options.c && !st->regex_error)
            output_line(buffer, filename, lines_count, st);
        free(buffer);
        lines_count++;
        match_count += match;
    }
    output_filename_and_count(filename, match_count, match, st);
}

bool find_match_in_line(char *line, struct llist *patterns, struct grep_state *st) {
    bool match = false;
    while (patterns != NULL && (!match || st->options.v) && !st->regex_error) {
        match |= find_match(line, patterns->data, st);
        patterns = patterns->next;
    }
    return (match ^ st->options.v) ^ st->regex_error;
}

bool find_match(char *line, char *pattern, struct grep_state *st) {
    regex_t re;
    int status = regcomp(&re, pattern, st->cflags | REG_NOSUB);
    if      (!*pattern)   status = 0;
    else if (status == 0) status = regexec(&re, line, 0, NULL, 0);
    else                  print_regex_error(status, &re);
    regfree(&re);
    st->regex_error = status != 0 && status != REG_NOMATCH;
    return status == 0;
}

// Searches not only for match, but for its offsets too
void search_substrings_in_file(FILE *f, char *filename, struct llist *patterns, struct grep_state *st) {
    size_t lines_count = 1;
    while (true) {
        char *buffer = NULL;
        ssize_t len = 0;
        size_t s = 0;
        if (!st->fatal_error || !st->regex_error) len = getline(&buffer, &s, f);
        struct offset_array pmatch_arr = { malloc(sizeof(regmatch_t)), 0 };

        st->fatal_error = pmatch_arr.data == NULL || buffer == NULL;
        if (len == -1 || st->fatal_error || st->regex_error) {
            free(pmatch_arr.data);
            free(buffer);
            break;
        }

        if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        bool match = find_substrings_in_line(buffer, patterns, st, &pmatch_arr);
        if (!st->fatal_error && !st->regex_error) {
            qsort(pmatch_arr.data, pmatch_arr.last_index, sizeof(regmatch_t), &regmatch_cmp);
            if (st->empty_pattern && !st->options.o)
                output_line(buffer, filename, lines_count, st);
            else if (match)
                output_substrings(buffer, filename, lines_count, &pmatch_arr, st);
        }
        free(pmatch_arr.data);
        free(buffer);
        lines_count++;
    }
}

bool find_substrings_in_line(char *line, struct llist *patterns,
                             struct grep_state *st, struct offset_array *pmatch_arr) {
    bool match = false;
    st->empty_pattern = false;
    while (patterns != NULL && !st->fatal_error && !st->regex_error) {
        match |= find_substrings(line, patterns->data, pmatch_arr, st);
        patterns = patterns->next;
    }
    return match;
}

bool find_substrings(char *line, char *pattern, struct offset_array *pmatch_arr, struct grep_state *st) {
    regex_t re;
    bool match = false;
    int status = regcomp(&re, pattern, st->cflags);
    if (!*pattern) {
        match = true;
    } else if (status == 0) {
        int i = 0;
        while (true) {
            int index = pmatch_arr->last_index;
            status = regexec(&re, line + i, 1, pmatch_arr->data + index, 0) || !line[i];
            match |= !status;
            regmatch_t *new_array = realloc(pmatch_arr->data, sizeof(regmatch_t) * (index + 2));
            if (new_array != NULL)
                pmatch_arr->data = new_array;
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
    } else {
        print_regex_error(status, &re);
    }
    regfree(&re);
    st->regex_error = status != 0 && status != REG_NOMATCH;
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
    if (!st->options.o) print_line_credentials(filename, line_number, st);
    int end = 0;
    for (size_t i = 0; i < pmatch_arr->last_index; i++) {
        if (pmatch_arr->data[i].rm_eo > end && pmatch_arr->data[i].rm_so >= end) {
            if (st->options.o) print_line_credentials(filename, line_number, st);
            while (end < pmatch_arr->data[i].rm_so && !st->options.o)
                putchar(line[end++]);
            int start = pmatch_arr->data[i].rm_so;
            end = pmatch_arr->data[i].rm_eo;
            // MATCH_HIGHLIGHT
            for (int j = start; j < end; j++)
                putchar(line[j]);
            // RESET
            if (st->options.o) putchar('\n');
        }
    }
    int len = strlen(line);
    while (end < len && !st->options.o)
        putchar(line[end++]);
    if (line[end - 1] != '\n' && !st->options.o)
        putchar('\n');
}

// Outputs filename and/or line number if corresponding flags and conditions are present
void print_line_credentials(char *filename, size_t line_number, struct grep_state *st) {
    if (!st->options.h && st->files_to_search > 1)
        printf("%s:", filename);
    if (st->options.n)
        printf("%zu:", line_number);
}

// Output for -l and -c flags
void output_filename_and_count(char *filename, size_t match_count, bool match,  struct grep_state *st) {
    if (match && st->options.l) {
        puts(filename);
    } else if ((match || !st->options.l) && st->options.c) {
        if (!st->options.h && st->files_to_search > 1) printf("%s:", filename);
        printf("%zu\n", match_count);
    }
}

// Comparator for regmatch offsets
int regmatch_cmp(const void *offset1, const void *offset2) {
    const regmatch_t *pmatch1 = offset1;
    const regmatch_t *pmatch2 = offset2;
    int result = (pmatch2->rm_so < pmatch1->rm_so) ||
                 (pmatch2->rm_so == pmatch1->rm_so && pmatch2->rm_eo > pmatch1->rm_eo);
    return result ? result : -1;
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
    fputc('\n', stderr);
}
