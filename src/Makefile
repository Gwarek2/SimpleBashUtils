ALL_C=$(shell find . -name "*.c")

all: s21_cat s21_grep

s21_cat:
	make -C cat s21_cat

s21_grep:
	make -C grep s21_grep

static_analysis:
	@-cp ../materials/linters/* .
	-python3 cpplint.py --extensions=c $(ALL_C)
	@-rm cpplint.py CPPLINT.cfg
	cppcheck --enable=all --suppress=missingIncludeSystem $(ALL_C)

clean:
	@-rm -rf *.o obj *.out *.dSYM
	make -C cat clean
	make -C grep clean