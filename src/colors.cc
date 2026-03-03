#include <cassert>

#include <unistd.h>
#include <stdio.h>

#include "colors.h"

int term_color_count = 0;

struct term_color term_colors[] = {
    { "RED"           , RED },
    { "GREEN"         , GREEN },
    { "YELLOW"        , YELLOW },
    { "BLUE"          , BLUE },
    { "PURPLE"        , PURPLE },
    { "CYAN"          , CYAN },
    { "WHITE"         , WHITE },
    { "DARK_GRAY"     , DARK_GRAY },
    { "BRIGHT_RED"    , BRIGHT_RED },
    { "BRIGHT_GREEN"  , BRIGHT_GREEN },
    { "BRIGHT_YELLOW" , BRIGHT_YELLOW },
    { "BRIGHT_BLUE"   , BRIGHT_BLUE },
    { "BRIGHT_PURPLE" , BRIGHT_PURPLE },
    { "BRIGHT_CYAN"   , BRIGHT_CYAN },
    { "BRIGHT_WHITE"  , BRIGHT_WHITE },
    { 0               , 0 }
};

void set_term_color_count() {
    term_color_count = 0;
    while (true) {
        if (!term_colors[term_color_count].name) {
            break;
        }
        term_color_count += 1;
    }
}

const char *get_term_escape(int column) {
    int idx = column % term_color_count;
    return term_colors[idx].ctlseq;
}

bool get_use_colors(int use_colors, FILE *out)
{
    switch (use_colors) {
    case COLOR_NO:
        return false;
    case COLOR_ALWAYS:
        return true;
    case COLOR_AUTO:
        return isatty(fileno(out));
    }
    assert(false);
}
