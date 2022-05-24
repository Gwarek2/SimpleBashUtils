CC=gcc
FLAGS=-Wall -Werror -Wextra -std=c11 #-g -fsanitize=address
CAT_FILES=../common/utils.c s21_cat.c

.PHONY: s21_cat
s21_cat: $(CAT_FILES)
	$(CC) $(FLAGS) $(CAT_FILES) -o s21_cat

clean:
	@-rm s21_cat