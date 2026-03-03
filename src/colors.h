#ifndef COLORS_H
#define COLORS_H

#define BLACK         "\x1b[0;22;30m"
#define RED           "\x1b[0;22;31m"
#define GREEN         "\x1b[0;22;32m"
#define YELLOW        "\x1b[0;22;33m"
#define BLUE          "\x1b[0;22;34m"
#define PURPLE        "\x1b[0;22;35m"
#define CYAN          "\x1b[0;22;36m"
#define WHITE         "\x1b[0;22;37m"
#define DARK_GRAY     "\x1b[0;1;30m"
#define BRIGHT_RED    "\x1b[0;1;31m"
#define BRIGHT_GREEN  "\x1b[0;1;32m"
#define BRIGHT_YELLOW "\x1b[0;1;33m"
#define BRIGHT_BLUE   "\x1b[0;1;34m"
#define BRIGHT_PURPLE "\x1b[0;1;35m"
#define BRIGHT_CYAN   "\x1b[0;1;36m"
#define BRIGHT_WHITE  "\x1b[0;1;37m"
#define TERM_RESET    "\x1b[0m"

#define COLOR_NO 0
#define COLOR_AUTO 1
#define COLOR_ALWAYS 2

struct term_color {
    const char *name;
    const char *ctlseq;
};

const char *get_term_escape(int);
bool get_use_colors(int, FILE *);
void set_protanomaly();

#endif
