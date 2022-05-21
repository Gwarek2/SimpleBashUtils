#ifndef S21_CAT
#define S21_CAT

#define LINE_N_FMT "    %lu  "

#define T_TAB "^I"

struct cat_state {
    struct {
        bool b_flag;
        bool e_flag;
        bool n_flag;
        bool s_flag;
        bool t_flag;
        bool v_flag;
    } flags;
    size_t line_count;
    bool nl_previous;
};

void initialize_state(struct cat_state *st);
void read_flags(struct cat_state *st, size_t argv, char *args[]);
void read_cat_flags(struct cat_state *st, char *str);
void execute_cat(char *args[], size_t argv, struct cat_state *st);
void print_file(const char *filename, struct cat_state *st);
int print_line(FILE *f_stream, struct cat_state *st);
inline void print_error();

#endif  // S21_CAT