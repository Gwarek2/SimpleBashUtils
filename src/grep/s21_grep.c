#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

#include "s21_grep.h"
#include "../common/utils.h"

int main(int argc, char *argv[]) {
    struct grep_state state;
    llist_t *regexes = initialize_llist();
    llist_t *files = initialize_llist();
    
    initialize_state(&state);
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
}

void parse_cmd_args(int argc, char *argv[], struct grep_state *st, llist_t *regexes, llist_t *files) {
    if (find_ef_flags(argc, argv)) {

    } else {

    }
}

bool find_ef_flags(int argc, char *argv[]) {
    bool result = false;
    for (size_t i; i < argc && !result; i++) {
        if (get_dash_index(argv[1]) == 1)
            result = strpbrk(argv[1], "ef") != NULL;
    }
    return result;
}

llist_t* initialize_llist() {
    llist_t *ll = malloc(sizeof(llist_t));
    ll->next = NULL;
    ll->data = NULL;
    return ll;
}

void add_to_llist(llist_t *ll, void *value) {
    llist_t *cursor = ll;
    while (cursor->next != NULL)
        cursor = cursor->next;
    cursor->data = value;
    cursor->next = malloc(sizeof(llist_t));
}

void free_list(llist_t *ll) {
    while (ll != NULL) {
        llist_t *tmp = ll;
        ll = ll->next;
        free(tmp);
    }
}