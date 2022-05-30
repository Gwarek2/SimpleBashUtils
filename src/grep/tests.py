#!/usr/bin/env python3

from itertools import permutations
import unittest
import os
import difflib

from requests import options

GREP_FILE = "grep.txt"
S21_GREP_FILE = "s21_grep.txt"
FILES_DIR = "../../datasets/grep"
REGEXES_DIR = "../../datasets/grep/regex"

REGEXES = {
    "1": "e",
    "2": "E",
    "email": "'[[:alnum:]]]*@[[[:alnum:]]]*\.[[[:alnum:]]*'",
    "ip-addres": "[[[:digit:]]]\{1,3\}.",
    "hex_number": "'[[[:digit:]a-fA-F]]]*'",
    "decimal_number": "[[[:digit:]]]",
    "octal_number": "'[[0-7]]*'",
    "binary_number": "'[[01]]*'",
    "empty": "''",
    "all": "[[[:alnum:]]]"
}

def get_test_files(dir):
    return tuple(
        os.path.join(dir, file) for file in
        filter(lambda file: file.endswith(".txt"), os.listdir(dir, ))
        )


def get_diff():
    with open(S21_GREP_FILE, "r") as f1, open(GREP_FILE, "r") as f2:
        s1 = f1.readlines()
        s2 = f2.readlines()
    return "".join(
        difflib.unified_diff(s1, s2, fromfile=S21_GREP_FILE, tofile=GREP_FILE)
    )


def execute_grep(*args):
    os.system(f"./s21_grep {' '.join(args)} > {S21_GREP_FILE}")
    os.system(f"grep {' '.join(args)} > {GREP_FILE}")


class GrepSingleOptionTestCase(unittest.TestCase):

    def setUp(self):
        self.test_files = get_test_files(FILES_DIR)
        self.regex_files = get_test_files(REGEXES_DIR)

    def test_no_option(self):
        for file in self.test_files:
            with self.subTest(i=file):
                execute_grep("e", file)
                diff = get_diff()
                self.assertFalse(diff, diff)
    
    def test_e_flag(self):
        for file in self.test_files:
            with self.subTest(i=file):
                execute_grep("e", file)
                diff = get_diff()
                self.assertFalse(diff, diff)
    
    def test_i_option(self):
        for file in self.test_files:
            with self.subTest(i=file):
                execute_grep("E", file, "-i")
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_v_option(self):
        for file in self.test_files:
            with self.subTest(i=file):
                execute_grep("e", "-v", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_c_option(self):
        for file in self.test_files:
            with self.subTest(i=file):
                execute_grep("e", "-c", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_l_option(self):
        for file in self.test_files:
            with self.subTest(i=file):
                execute_grep("e", "-l", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_n_option(self):
        for file in self.test_files:
            with self.subTest(i=file):
                execute_grep("e", "-n", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_h_option(self):
        execute_grep("e", "-h", *self.test_files)
        diff = get_diff()
        self.assertFalse(diff, diff)

    def test_s_option_nonexisting_files(self):
        execute_grep("e", "-s", "aboba", "nwah", *self.test_files)
        diff = get_diff()
        self.assertFalse(diff, diff)

    def test_s_option_existing_files(self):
        execute_grep("e", "-s", *self.test_files)
        diff = get_diff()
        self.assertFalse(diff, diff)

    def test_o_option(self):
        for file in self.test_files:
            with self.subTest(i=file):
                execute_grep("e", "-o", file)
                diff = get_diff()
                self.assertFalse(diff, diff)

    def test_o_option_several_patterns(self):
        for file in self.test_files:
            with self.subTest(file=file):
                for regexes in permutations(REGEXES.values(), 2):
                    with self.subTest(regexes=regexes):
                        regs = ("-e", regexes[0], "-e", regexes[1])
                        execute_grep(*regs, "-o", file)
                        diff = get_diff()
                        self.assertFalse(diff, diff)
    
    def test_f_option(self):
        for file in self.test_files:
            with self.subTest(file=file):
                for regfile in self.regex_files:
                    with self.subTest(regfile=regfile):
                        execute_grep("-f", regfile, file)
                        diff = get_diff()
                        self.assertFalse(diff, diff)


class GrepMultipleOptionsTestCase(unittest.TestCase):

    options = ("-i", "-v", "-c", "-l", "-n", "-h", "-s", "-o")

    def setUp(self):
        self.test_files = get_test_files(FILES_DIR)

    def test_two_options(self):
        for file in self.test_files:
            with self.subTest(file=file):
                for opts in permutations(self.options, 2):
                    with self.subTest(options=opts):
                        if ('-o' in opts and '-v' in opts):
                            continue
                        execute_grep('-e', REGEXES["all"], *opts, file)
                        diff = get_diff()
                        self.assertFalse(diff, diff)

if __name__ == "__main__":
    unittest.main()