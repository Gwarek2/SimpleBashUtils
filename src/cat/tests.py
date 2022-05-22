import unittest
import os
import difflib

CAT_FILE = "cat.txt"
S21_CAT_FILE = "s21_cat.txt"
FILES_DIR = "../../datasets"

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


class CatSingleOptionsTestCase(unittest.TestCase):

    def setUp(self):
        self.test_files = get_test_files();

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

    def test_gnu_s_option(self):
        for file in self.test_files:
            execute_cat(file, '--squeeze-blank')
            diff = get_diff()
            self.assertFalse(diff, diff)

    def test_v_option(self):
        for file in self.test_files:
            execute_cat(file, '-v')
            diff = get_diff()
            self.assertFalse(diff, diff)

    def tearDown(self):
        os.remove(S21_CAT_FILE)
        os.remove(CAT_FILE)


# class CatMultipleOptionsTestCase(unittest.TestCase):

#     def setUp(self):
#         self.test_files = get_test_files();

    


if __name__ == "__main__":
    unittest.main()