#ifndef GREP
#define GREP


#define GREP_DEFAULT { \
    { false,  }, \
    0, false, false, false, 0, 0, 0 \
};

#define MATCH_HIGHLIGHT printf("\x1b[31m");
#define RESET           printf("\x1b[0m");


struct grep_state {
    struct {
        bool e, v, c, l, n, h, s, f, o;
    } options;
    int cflags;
    bool empty_pattern;
    bool fatal_error;
    bool regex_error;
    size_t files_to_search;
    size_t match_count;
    size_t first_regex_index;
};

typedef struct llist_t {
    struct llist_t *next;
    void *data;
    bool heap_used;
} llist_t;

struct offset_array {
    regmatch_t *data;
    size_t last_index;
};

void parse_cmd_args(int argc, char *argv[], struct grep_state *st, llist_t *regexes, llist_t *files);
void parse_regexes(int argc, char *argv[], llist_t *regexes, struct grep_state *st);
llist_t* read_regex_from_file(char *filename, llist_t *regexes);
void parse_filenames(int argc, char *argv[], llist_t *files, struct grep_state *st);
void parse_options(int argc, char *argv[], struct grep_state *st);

void process_files(llist_t *patterns, llist_t *files, struct grep_state *st);
void process_stdio(llist_t *patterns, struct grep_state *st);

void search_matches_in_file(FILE *f, char *filename, llist_t *patterns, struct grep_state *st);
bool find_match_in_line(char *line, llist_t *patterns, struct grep_state *st);
bool find_match(char *line, char *pattern, struct grep_state *st);

void search_substrings_in_file(FILE *f, char *filename, llist_t *patterns, struct grep_state *st);
bool find_substrings_in_line(char *line, llist_t *patterns, struct grep_state *st, struct offset_array *pmatch_arr);
bool find_substrings(char *line, char *pattern, struct offset_array *pmatch_arr, struct grep_state *st);

void output_line(char *line, char *filename, size_t line_number, struct grep_state *st);
void output_substrings(char *line, char *filename, size_t linse_number, struct offset_array *pmatch_arr, struct grep_state *st);
void output_filename_and_count(char *filename, size_t match_count, bool match, struct grep_state *st);
void print_line_credentials(char *filename, size_t line_number, struct grep_state *st);

int regmatch_cmp(const void *offset1, const void *offset2);

void print_regex_error(int err_code, regex_t *re);

llist_t* initialize_llist();
llist_t* add_to_llist(llist_t *ll, void *value, bool heap_used);
void free_llist(llist_t *ll);

#endif  // GREP
