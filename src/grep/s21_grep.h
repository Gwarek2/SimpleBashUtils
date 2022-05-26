#ifndef GREP
#define GREP


#define GREP_DEFAULT {\
    { false, false, false, false, false, false, false, false, false, false },\
    0, 0\
};


struct grep_state {
    struct {
        bool e;
        bool i;
        bool v;
        bool c;
        bool l;
        bool n;
        bool h;
        bool s;
        bool f;
        bool o;
    } flags;
    size_t files_to_search;
    size_t match_count;
};

struct llist {
    struct llist *next;
    void *data;
    bool heap_used;
};

struct offset_array {
    regmatch_t *data;
    size_t last_index;
};

void initialize_state(struct grep_state *st);
void parse_cmd_args(int argc, char *argv[], struct grep_state *st, struct llist *regexes, struct llist *files);
void parse_regexes(int argc, char *argv[], struct llist *regexes, struct grep_state *st);
void read_regex_from_file(char *filename, struct llist *regexes);
void parse_files(int argc, char *argv[], struct llist *files, struct grep_state *st);
void parse_options(int argc, char *argv[], struct grep_state *st);

void process_files(struct llist *patterns, struct llist *files, struct grep_state *st);
void search_in_file(FILE *f, char *filename, struct llist *patterns, struct grep_state *st);
bool find_patterns_in_line(char *line, struct llist *patterns, struct grep_state *st);
bool find_match(char *line, char *pattern, struct grep_state *st);
bool find_offsets(char *line, char *pattern, struct offset_array *pmatch_arr, struct grep_state *st);
void output_line(char *line, char *filename, size_t line_number, struct grep_state *st);
void output_filename_and_count(char *filename, size_t match_count, struct grep_state *st);

void print_regex_error(int err_code, regex_t *re);

struct llist* initialize_llist();
void add_to_llist(struct llist *ll, void *value, bool heap_used);
void free_llist(struct llist *ll);

#endif  // GREP