CC=gcc
FLAGS=-Wall -Werror -Wextra -std=c11 -g -fsanitize=address
GREP_FILES=s21_grep.c s21_grep.h

.PHONY: s21_grep
s21_grep: $(GREP_FILES)
	$(CC) $(FLAGS) s21_grep.c -o s21_grep

clean:
	@-rm s21_grep