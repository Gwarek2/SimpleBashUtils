#!/usr/bin/env python3

from itertools import permutations
import unittest
import os
import difflib


GREP_FILE = "grep.txt"
S21_GREP_FILE = "s21_grep.txt"
S21_ERR = "s21_err.txt"
ERR = "err.txt"
FILES_DIR = "../../datasets/grep"
REGEXES_DIR = "../../datasets/grep/regex"

REGEXES = {
    "1": "e",
    "2": "E",
    "3": "commit",
    "5": "Lorem",
    "6": "rotec",
    "hex": "'[[:digit:]a-fA-F]]\\{1,\\}'",
    "decimal": "'[[:digit:]]\\{1,\\}'",
    "octal": "'[0-7]\\{1,\\}'",
    "binary": "'[01]\\{1,\\}'",
    "empty": "''",
    "all": "'[[:alnum:]]\\{1,\\}'"
}

def get_test_files(dir):
    return tuple(
        os.path.join(dir, file) for file in
        filter(lambda file: file.endswith(".txt"), os.listdir(dir, ))
        )


def get_diff(file1=S21_GREP_FILE, file2=GREP_FILE):
    with open(file1, "r") as f1, open(file2, "r") as f2:
        s1 = f1.readlines()
        s2 = f2.readlines()
    return "".join(
        difflib.unified_diff(s1, s2, fromfile=file1, tofile=file2)
    )


def execute_grep(*args):
    os.system(f"./s21_grep {' '.join(args)} > {S21_GREP_FILE} 2> {S21_ERR}")
    os.system(f"grep {' '.join(args)} > {GREP_FILE} 2> {ERR}")


class GrepSingleOptionTestCase(unittest.TestCase):

    def setUp(self):
        self.t_files = get_test_files(FILES_DIR)
        self.regex_files = get_test_files(REGEXES_DIR)
        self.regexes = " -e ".join(REGEXES.values())

    def test_no_option(self):
        for file in self.t_files:
            with self.subTest(file=file):
                execute_grep("e", file)
                diff = get_diff()
                self.assertFalse(diff, diff)
    
    def test_e_flag(self):
        for file in self.t_files:
            with self.subTest(i=file):
                execute_grep("e", file)
                diff = get_diff()
                self.assertFalse(diff, diff)
    
    def test_i_option(self):
        for file in self.t_files:
            with self.subTest(file=file):
                execute_grep("E", file, "-i")
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_v_option(self):
        for file in self.t_files:
            with self.subTest(file=file):
                execute_grep("e", "-v", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_c_option(self):
        for file in self.t_files:
            with self.subTest(i=file):
                execute_grep("e", "-c", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_l_option(self):
        for file in self.t_files:
            with self.subTest(file=file):
                execute_grep("e", "-l", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_n_option(self):
        for file in self.t_files:
            with self.subTest(file=file):
                execute_grep("e", "-n", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_h_option(self):
        execute_grep("e", "-h", *self.t_files)
        diff = get_diff()
        self.assertFalse(diff, diff)

    def test_s_option_nonexisting_files(self):
        execute_grep("e", "-s", "aboba", "nwah", *self.t_files)
        diff = get_diff()
        self.assertFalse(diff, diff)

    def test_s_option_existing_files(self):
        execute_grep("e", "-s", *self.t_files)
        diff = get_diff()
        self.assertFalse(diff, diff)
        err_diff = get_diff(S21_ERR, ERR)
        self.assertFalse(err_diff, err_diff)

    def test_o_option(self):
        for file in self.t_files:
            with self.subTest(file=file):
                execute_grep("e", "-o", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_o_option_several_patterns(self):
        for file in self.t_files:
            with self.subTest(file=file):
                for regexes in permutations(REGEXES.values(), 2):
                    with self.subTest(regexes=regexes):
                        regs = ("-e", regexes[0], "-e", regexes[1])
                        execute_grep(*regs, "-o", file)
                        diff = get_diff()
                        self.assertFalse(diff, diff)
    
    def test_f_option(self):
        for file in self.t_files:
            with self.subTest(file=file):
                for regfile in self.regex_files:
                    with self.subTest(regfile=regfile):
                        execute_grep("-f", regfile, file)
                        diff = get_diff()
                        self.assertFalse(diff, diff)

    def test_f_option_multiple_files(self):
        execute_grep("-f", " -f ".join(self.regex_files), *self.t_files)
        diff = get_diff()
        self.assertFalse(diff, diff)

    def tearDown(self):
        os.remove(S21_GREP_FILE)
        os.remove(GREP_FILE)
        os.remove(S21_ERR)
        os.remove(ERR)


class GrepMultipleOptionsTestCase(unittest.TestCase):

    options = ("-i", "-v", "-c", "-l", "-n", "-h", "-s", "-o")

    def setUp(self):
        self.t_files = get_test_files(FILES_DIR)
        self.regexes = " -e ".join(REGEXES.values())

    def test_two_options(self):
        for file in self.t_files:
            with self.subTest(file=file):
                for opts in permutations(self.options, 2):
                    with self.subTest(options=opts, regex=self.regexes):
                        if ('-o' in opts and '-v' in opts):
                            continue
                        execute_grep('-e', self.regexes, *opts, file, "nwah", "blahblah")
                        diff = get_diff()
                        self.assertFalse(diff, diff)
                        err_diff = get_diff(S21_ERR, ERR)
                        self.assertFalse(err_diff, err_diff)
    
    def tearDown(self):
        os.remove(S21_GREP_FILE)
        os.remove(GREP_FILE)
        os.remove(S21_ERR)
        os.remove(ERR)



class GrepMultipleOptionsAndFilesTestCase(unittest.TestCase):

    options = ("-i", "-v", "-c", "-l", "-n", "-h", "-s", "-o")

    def setUp(self):
        self.t_files = get_test_files(FILES_DIR)
        self.regexes = " -e ".join(REGEXES.values())


    def test_two_options(self):
        for opts in permutations(self.options, 2):
            with self.subTest(options=opts, regex=self.regexes):
                if ('-o' in opts and '-v' in opts):
                    continue
                execute_grep('-e', self.regexes, *opts, ' '.join(self.t_files), "nwah", "blahblah")
                diff = get_diff()
                self.assertFalse(diff, diff)
                err_diff = get_diff(S21_ERR, ERR)
                self.assertFalse(err_diff, err_diff)

    def tearDown(self):
        os.remove(S21_GREP_FILE)
        os.remove(GREP_FILE)
        os.remove(S21_ERR)
        os.remove(ERR)


if __name__ == "__main__":
    log_file = "result.txt"
    with open(log_file, "w") as f:
        tr = unittest.TextTestRunner(f)
        unittest.main(testRunner=tr)
