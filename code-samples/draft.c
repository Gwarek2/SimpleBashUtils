#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

int main()
{
    char *str = "string straight";
    char *pat = "str";
    regex_t re;
    regcomp(&re, pat, REG_ICASE);
    int res = 0;
    int i = 0;
    int j = 0;
    regmatch_t **pmatch_arr = malloc(sizeof(regmatch_t*));
    
    while (1) {
        regmatch_t *pmatch = malloc(sizeof(regmatch_t));
        res = regexec(&re, str + i, 1, pmatch, 0);
        if (res != 0) break;
        // printf("\x1b[31m" "%i %i\n" "\x1b[0m", i + pmatch->rm_so, i + pmatch->rm_eo);
        i += pmatch[0].rm_eo - 1;
        pmatch_arr[j++] = pmatch;
        pmatch_arr = realloc(pmatch_arr, sizeof(regmatch_t*) * (j + 1));
    }
    for (int k = 0; k < j; k++) {
        printf("%i %i\n", pmatch_arr[k]->rm_so, pmatch_arr[k]->rm_eo);
        free(pmatch_arr[k]);
    }
    regfree(&re);
    free(pmatch_arr);
    return 0;
}