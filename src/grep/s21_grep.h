#ifndef GREP
#define GREP


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

void initialize_state(struct grep_state *st);
void parse_cmd_args(int argc, char *argv[], struct grep_state *st, struct llist *regexes, struct llist *files);
void parse_regexes(int argc, char *argv[], struct llist *regexes, struct grep_state *st);
void read_regex_from_file(char *filename, struct llist *regexes);
void parse_files(int argc, char *argv[], struct llist *files);
void parse_options(int argc, char *argv[], struct grep_state *st);

struct llist* initialize_llist();
void add_to_llist(struct llist *ll, void *value, bool heap_used);
void free_list(struct llist *ll);

#endif  // GREP