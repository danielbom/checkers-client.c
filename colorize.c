#ifndef COLORIZE
#define COLORIZE

// https://www.dreamincode.net/forums/topic/25873-changing-terminals-bg-color-in-c/
char RED_COLOR[] = "\033[0;31m";
char GREEN_COLOR[] = "\033[0;32m";
char YELLOW_COLOR[] = "\033[0;33m";
char BLUE_COLOR[] = "\033[0;34m";
char MAGENTA_COLOR[] = "\033[0;35m";
char CYAN_COLOR[] = "\033[0;36m";
char BOLD_RED_COLOR[] = "\033[1;31m";
char BOLD_GREEN_COLOR[] = "\033[1;32m";
char BOLD_YELLOW_COLOR[] = "\033[1;33m";
char BOLD_BLUE_COLOR[] = "\033[1;34m";
char BOLD_MAGENTA_COLOR[] = "\033[1;35m";
char BOLD_CYAN_COLOR[] = "\033[1;36m";

char BG_BLACK_COLOR[] = "\33[40m";
char BG_RED_COLOR[] = "\033[41m";
char BG_GREEN_COLOR[] = "\033[42m";
char BG_YELLOW_COLOR[] = "\033[43m";
char BG_BLUE_COLOR[] = "\033[44m";
char BG_MAGENTA_COLOR[] = "\033[45m";
char BG_CYAN_COLOR[] = "\033[46m";
char BG_WHITE_COLOR[] = "\33[47m";

char RESET_COLOR[] = "\033[0m";
char BLINK_COLOR[] = "\033[5m";
char REVERSE_COLOR[] = "\033[7m";
char BLINK_OFF_COLOR[] = "\033[25m";
char REVERSE_OFF_COLOR[] = "\033[27m";

#endif