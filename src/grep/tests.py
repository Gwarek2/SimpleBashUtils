from itertools import permutations
import unittest
import os
import sys
import difflib

GREP_FILE = "grep.txt"
S21_GREP_FILE = "s21_grep.txt"
FILES_DIR = "../../datasets/grep"

REGEXES = {
    "email": "[[:word:]]*@[[:word:]]*\.[[:word:]]*",
    "ip-addres": "[[:digit:]]\{1,3\}.",
    "just_word": "[[:word:]]*",
    "hex_number": "[[:digit:]a-f]]",
}

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
