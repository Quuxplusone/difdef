#ifndef COLORS_H
#define COLORS_H

#define COLOR_NORMAL         ""
#define COLOR_RESET          "\033[0;m"
#define COLOR_BOLD           "\033[0;1m"
#define COLOR_RED            "\033[0;31m"
#define COLOR_GREEN          "\033[0;32m"
#define COLOR_YELLOW         "\033[0;33m"
#define COLOR_BLUE           "\033[0;34m"
#define COLOR_MAGENTA        "\033[0;35m"
#define COLOR_CYAN           "\033[0;36m"
#define COLOR_WHITE          "\033[0;37n"
#define COLOR_BRIGHT_RED     "\033[0;1;31m"
#define COLOR_BRIGHT_GREEN   "\033[0;1;32m"
#define COLOR_BRIGHT_YELLOW  "\033[0;1;33m"
#define COLOR_BRIGHT_BLUE    "\033[0;1;34m"
#define COLOR_BRIGHT_MAGENTA "\033[0;1;35m"
#define COLOR_BRIGHT_CYAN    "\033[0;1;36m"
#define COLOR_BRIGHT_WHITE   "\033[0;1;37m"
#define COLOR_FAINT_RED      "\033[0;2;31m"
#define COLOR_FAINT_GREEN    "\033[0;2;32m"
#define COLOR_FAINT_YELLOW   "\033[0;2;33m"
#define COLOR_FAINT_BLUE     "\033[0;2;34m"
#define COLOR_FAINT_MAGENTA  "\033[0;2;35m"
#define COLOR_FAINT_CYAN     "\033[0;2;36m"
#define COLOR_FAINT_WHITE    "\033[0;2;37m"
#define COLOR_BG_RED         "\033[0;41m"
#define COLOR_BG_GREEN       "\033[0;42m"
#define COLOR_BG_YELLOW      "\033[0;43m"
#define COLOR_BG_BLUE        "\033[0;44m"
#define COLOR_BG_MAGENTA     "\033[0;45m"
#define COLOR_BG_CYAN        "\033[0;46m"
#define COLOR_BG_WHITE       "\033[0;47m"
#define COLOR_FAINT          "\033[0;2m"
#define COLOR_FAINT_ITALIC   "\033[0;2;3m"
#define COLOR_REVERSE        "\033[0;7m"

#define COLOR_USE_NO 0
#define COLOR_USE_AUTO 1
#define COLOR_USE_ALWAYS 2

struct term_color {
    const char *name;
    const char *ctlseq;
};

const char *get_term_escape(int);
bool get_use_colors(int, FILE *);
void set_protanomaly();

#endif
