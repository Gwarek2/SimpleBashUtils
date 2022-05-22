CC=gcc
FLAGS=-Wall -Werror -Wextra -std=c11 -g -fsanitize=address
CAT_FILES=cat/s21_cat.c cat/s21_cat.h
# CHECKMK=checkmk clean_mode=1
# OUT_DIR=obj
# COV_DIR=coverage_info
ALL_C=$(shell find . -name "*.c")
# OBJ=$(shell find . -name "*.c" | sed "s/lib/obj/" | sed "s/\.c/\.o/")
# CHECK_FLAGS=$(shell pkg-config --cflags --libs check)

.PHONY: s21_cat
s21_cat: $(CAT_FILES)
	$(CC) $(FLAGS) cat/s21_cat.c -o cat/s21_cat

s21_grep:

static_analysis:
	@-cp ../materials/linters/* .
	python3 cpplint.py --extensions=c $(ALL_C)
	@-rm cpplint.py CPPLINT.cfg
	cppcheck $(ALL_C)

clean:
	@-rm -rf *.o obj *.out *.dSYM s21_cat