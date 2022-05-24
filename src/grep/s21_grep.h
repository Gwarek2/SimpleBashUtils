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
};

typedef struct {
    llist_t *next;
    void *data;
} llist_t;

void initialize_state(struct grep_state *st);

#endif  // GREP