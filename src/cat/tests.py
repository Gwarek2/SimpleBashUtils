#!/usr/bin/env python3

from itertools import permutations
import unittest
import os
import sys
import difflib

CAT_FILE = "cat.txt"
S21_CAT_FILE = "s21_cat.txt"
FILES_DIR = "../../datasets/cat"

def get_test_files():
    return tuple(
        os.path.join(FILES_DIR, file) for file in
        filter(lambda file: file.endswith(".txt"), os.listdir(FILES_DIR, ))
        )


def get_diff():
    with open(S21_CAT_FILE, "r") as f1, open(CAT_FILE, "r") as f2:
        s1 = f1.readlines()
        s2 = f2.readlines()
    return "".join(
        difflib.unified_diff(s1, s2, fromfile=S21_CAT_FILE, tofile=CAT_FILE)
    )

def execute_cat(file, args):
    os.system(f"./s21_cat {args} {file} > {S21_CAT_FILE}")
    os.system(f"cat {args} {file} > {CAT_FILE}")


def skip_if_not_gnu(test_func):
    if sys.platform == "linux":
        return test_func
    return None


class CatSingleOptionsTestCase(unittest.TestCase):

    def setUp(self):
        self.test_files = get_test_files()

    def test_no_option(self):
        for file in self.test_files:
            execute_cat(file, '')
            diff = get_diff()
            self.assertFalse(diff, diff)

    def test_n_option(self):
        for file in self.test_files:
            execute_cat(file, '-n')
            diff = get_diff()
            self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_gnu_n_option(self):
        for file in self.test_files:
            execute_cat(file, '--number')
            diff = get_diff()
            self.assertFalse(diff, diff)

    def test_b_option(self):
        for file in self.test_files:
            execute_cat(file, '-b')
            diff = get_diff()
            self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_gnu_b_option(self):
        for file in self.test_files:
            execute_cat(file, '--number-nonblank')
            diff = get_diff()
            self.assertFalse(diff, diff)

    def test_e_option(self):
        for file in self.test_files:
            execute_cat(file, '-e')
            diff = get_diff()
            self.assertFalse(diff, diff)
    
    @skip_if_not_gnu
    def test_gnu_E_option(self):
        for file in self.test_files:
            execute_cat(file, '-E')
            diff = get_diff()
            self.assertFalse(diff, diff)
    
    def test_t_option(self):
        for file in self.test_files:
            execute_cat(file, '-t')
            diff = get_diff()
            self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_gnu_T_option(self):
        for file in self.test_files:
            execute_cat(file, '-T')
            diff = get_diff()
            self.assertFalse(diff, diff)

    def test_s_option(self):
        for file in self.test_files:
            execute_cat(file, '-s')
            diff = get_diff()
            self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_gnu_s_option(self):
        for file in self.test_files:
            execute_cat(file, '--squeeze-blank')
            diff = get_diff()
            self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_v_option(self):
        for file in self.test_files:
            execute_cat(file, '-v')
            diff = get_diff()
            self.assertFalse(diff, diff)

    def tearDown(self):
        os.remove(S21_CAT_FILE)
        os.remove(CAT_FILE)


class CatMultipleOptionsTestCase(unittest.TestCase):

    all_flags = ('b', 'e', 'n', 's', 't')
    if sys.platform == "linux":
        all_flags += ("-E", "-T", "-v", "--number-nonblank", "--number", "--squeeze-blank")

    def setUp(self):
        self.test_files = get_test_files()
    
    def test_two_flags(self):
        for file in self.test_files:
            for flags in permutations(self.all_flags, 2):
                execute_cat(file, '-' + ''.join(flags))
                diff = get_diff()
                self.assertFalse(
                    diff, f"\nflags:\n {flags}\n{diff}"
                )
    
    def test_three_flags(self):
        for file in self.test_files:
            for flags in permutations(self.all_flags, 3):
                execute_cat(file, '-' + ''.join(flags))
                diff = get_diff()
                self.assertFalse(
                    diff, f"\nflags:\n {flags}\n{diff}"
                )

    def test_four_flags(self):
        for file in self.test_files:
            for flags in permutations(self.all_flags, 4):
                execute_cat(file, '-' + ''.join(flags))
                diff = get_diff()
                self.assertFalse(
                    diff, f"\nflags:\n {flags}\n{diff}"
                )

    def test_all_flags(self):
        for file in self.test_files:
            execute_cat(file, '-' + "".join(self.all_flags))
            diff = get_diff()
            self.assertFalse(diff, diff)

    def tearDown(self):
        os.remove(S21_CAT_FILE)
        os.remove(CAT_FILE)


class CatMultipleFilesTestCase(unittest.TestCase):

    def setUp(self):
        self.test_files = " ".join(get_test_files())

    def test_no_option(self):
        execute_cat(self.test_files, '')
        diff = get_diff()
        self.assertFalse(diff, diff)

    def test_n_option(self):
        execute_cat(self.test_files, '-n')
        diff = get_diff()
        self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_gnu_n_option(self):
        execute_cat(self.test_files, '--number')
        diff = get_diff()
        self.assertFalse(diff, diff)

    def test_b_option(self):
        execute_cat(self.test_files, '-b')
        diff = get_diff()
        self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_gnu_b_option(self):
        execute_cat(self.test_files, '--number-nonblank')
        diff = get_diff()
        self.assertFalse(diff, diff)

    def test_e_option(self):
        execute_cat(self.test_files, '-e')
        diff = get_diff()
        self.assertFalse(diff, diff)
    
    @skip_if_not_gnu
    def test_gnu_E_option(self):
        execute_cat(self.test_files, '-E')
        diff = get_diff()
        self.assertFalse(diff, diff)
    
    def test_t_option(self):
        execute_cat(self.test_files, '-t')
        diff = get_diff()
        self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_gnu_T_option(self):
        execute_cat(self.test_files, '-T')
        diff = get_diff()
        self.assertFalse(diff, diff)

    def test_s_option(self):
        execute_cat(self.test_files, '-s')
        diff = get_diff()
        self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_gnu_s_option(self):
        execute_cat(self.test_files, '--squeeze-blank')
        diff = get_diff()
        self.assertFalse(diff, diff)

    @skip_if_not_gnu
    def test_v_option(self):
        execute_cat(self.test_files, '-v')
        diff = get_diff()
        self.assertFalse(diff, diff)

    def tearDown(self):
        os.remove(S21_CAT_FILE)
        os.remove(CAT_FILE)


if __name__ == "__main__":
    unittest.main()