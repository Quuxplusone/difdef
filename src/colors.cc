#include <cassert>

#include <unistd.h>
#include <stdio.h>

#include "colors.h"

int term_color_count = 0;
int term_color_protanomaly = 1;

struct term_color term_colors[] = {
    { "RED"           , RED },
    { "GREEN"         , GREEN },
    { "YELLOW"        , YELLOW },
    { "BLUE"          , BLUE },
    { "PURPLE"        , PURPLE },
    { "CYAN"          , CYAN },
    { "WHITE"         , WHITE },
    { "BRIGHT_RED"    , BRIGHT_RED },
    { "BRIGHT_GREEN"  , BRIGHT_GREEN },
    { "BRIGHT_YELLOW" , BRIGHT_YELLOW },
    { "BRIGHT_BLUE"   , BRIGHT_BLUE },
    { "BRIGHT_PURPLE" , BRIGHT_PURPLE },
    { "BRIGHT_CYAN"   , BRIGHT_CYAN },
    { 0               , 0 }
};

struct term_color term_colors_protanomaly[] = {
    { "YELLOW"        , YELLOW },
    { "BLUE"          , BLUE },
    { "WHITE"         , WHITE },
    { "BRIGHT_YELLOW" , BRIGHT_YELLOW },
    { "BRIGHT_BLUE"   , BRIGHT_BLUE },
    { "BRIGHT_WHITE"  , BRIGHT_WHITE },
    { 0               , 0 }
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define COLOR_COUNT (ARRAY_SIZE(term_colors) - 1)
#define COLOR_COUNT_PROTANOMALY (ARRAY_SIZE(term_colors_protanomaly) - 1)

const char *get_term_escape(int column) {
    if (term_color_protanomaly) {
        int idx = column % COLOR_COUNT_PROTANOMALY;
        return term_colors_protanomaly[idx].ctlseq;
    }
    int idx = column % COLOR_COUNT;
    return term_colors[idx].ctlseq;
}

void set_protanomaly() {
    term_color_protanomaly = 1;
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
