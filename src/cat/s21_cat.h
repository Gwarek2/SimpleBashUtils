#ifndef S21_CAT
#define S21_CAT

#define LINE_N_FMT "    %lu  "

#define T_TAB "^I"
#define M_NOT "M-"

struct cat_state {
    struct {
        bool b_flag;
        bool e_flag;
        bool n_flag;
        bool s_flag;
        bool t_flag;
        bool v_flag;
    } flags;
    size_t line_count;
    bool nl_previous;
    bool filenames;
};

void initialize_state(struct cat_state *st);
void read_flags(struct cat_state *st, size_t argv, char *args[]);
void read_cat_flags(struct cat_state *st, char *str);
void execute_cat_files(char *args[], size_t argv, struct cat_state *st);
void execute_cat_stdin(struct cat_state *st);
void print_file(const char *filename, struct cat_state *st);
int print_line(FILE *f_stream, struct cat_state *st);
void print_v_format(int ch);
inline void print_error();

const char V_FORMAT[128][3] = {
    [0] = "^@",    [1] = "^A",    [2] = "^B",    [3] = "^C",
    [4] = "^D", [5] = "^E",    [6] = "^F",    [7] = "^G",
    [8] = "^H", [9] = "", [10] = "",   [11] = "^K",
    [12] = "^L", [13] = "^M",  [14] = "^N",   [15] = "^O",
    [16] = "^P",   [17] = "^Q",   [18] = "^R",   [19] = "^S",
    [20] = "^T",   [21] = "^U",   [22] = "^V",   [23] = "^W",
    [24] = "^X",   [25] = "^Y",   [26] = "^Z",   [27] = "^[",
    [28] = "^\\", [29] = "^]", [30] = "^^",   [31] = "^_",
    [32] = " ", [33] = "!", [34] = "\"", [35] = "#", 
    [36] = "$", [37] = "%", [38] = "&", [39] = "'", 
    [40] = "(", [41] = ")", [42] = "*", [43] = "+", 
    [44] = ",", [45] = "-", [46] = ".", [47] = "/", 
    [48] = "0", [49] = "1", [50] = "2", [51] = "3", 
    [52] = "4", [53] = "5", [54] = "6", [55] = "7", 
    [56] = "8", [57] = "9", [58] = ":", [59] = ";", 
    [60] = "<", [61] = "=", [62] = ">", [63] = "?", 
    [64] = "@", [65] = "A", [66] = "B", [67] = "C", 
    [68] = "D", [69] = "E", [70] = "F", [71] = "G", 
    [72] = "H", [73] = "I", [74] = "J", [75] = "K", 
    [76] = "L", [77] = "M", [78] = "N", [79] = "O", 
    [80] = "P", [81] = "Q", [82] = "R", [83] = "S", 
    [84] = "T", [85] = "U", [86] = "V", [87] = "W", 
    [88] = "X", [89] = "Y", [90] = "Z", [91] = "[", 
    [92] = "\\", [93] = "]", [94] = "^", [95] = "_", 
    [96] = "`", [97] = "a", [98] = "b", [99] = "c", 
    [100] = "d", [101] = "e", [102] = "f", [103] = "g", 
    [104] = "h", [105] = "i", [106] = "j", [107] = "k", 
    [108] = "l", [109] = "m", [110] = "n", [111] = "o", 
    [112] = "p", [113] = "q", [114] = "r", [115] = "s", 
    [116] = "t", [117] = "u", [118] = "v", [119] = "w", 
    [120] = "x", [121] = "y", [122] = "z", [123] = "{", 
    [124] = "|", [125] = "}", [126] = "~", [127] = "^?", 
};

#endif  // S21_CAT