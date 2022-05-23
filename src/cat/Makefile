CC=gcc
FLAGS=-Wall -Werror -Wextra -std=c11 #-g -fsanitize=address
CAT_FILES=s21_cat.c s21_cat.h

.PHONY: s21_cat
s21_cat: $(CAT_FILES)
	$(CC) $(FLAGS) s21_cat.c -o s21_cat

clean:
	@-rm s21_cat