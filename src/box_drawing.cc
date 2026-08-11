#include <stdlib.h>
#include <string.h>

#include "box_drawing.h"

void init_box_drawing(bool force_ascii) {
    box_drawing_vertical = (char *)"|";
    box_drawing_horizontal = (char *)"-";
    box_drawing_vert_horiz = (char *)"|";
    box_drawing_horiz_vert = (char *)"-";
    box_drawing_right_down = (char *)".";
    box_drawing_right_up = (char *)"'";
    box_drawing_horiz_up = (char *)"'";
    box_drawing_horiz_down = (char *)".";
    if (force_ascii) {
        return;
    }
    char *locale = getenv("LANG");
    if (locale == NULL) {
        return;
    }
    char *suffix = ".UTF-8";
    if (strlen(locale) <= strlen(suffix)) {
        return;
    }
    char *p = locale + strlen(locale) - strlen(suffix);
    if (!strcmp(p, suffix)) {
        box_drawing_vertical   = (char *)malloc(4);
        box_drawing_horizontal = (char *)malloc(4);
        box_drawing_vert_horiz = (char *)malloc(4);
        box_drawing_horiz_vert = (char *)malloc(4);
        box_drawing_right_down = (char *)malloc(4);
        box_drawing_right_up   = (char *)malloc(4);
        box_drawing_horiz_down = (char *)malloc(4);
        box_drawing_horiz_up   = (char *)malloc(4);

        box_drawing_vertical[0]   = UTF_8_BYTE_1(BOX_DRAWING_VERTICAL);
        box_drawing_horizontal[0] = UTF_8_BYTE_1(BOX_DRAWING_HORIZONTAL);
        box_drawing_vert_horiz[0] = UTF_8_BYTE_1(BOX_DRAWING_VERT_HORIZ);
        box_drawing_horiz_vert[0] = UTF_8_BYTE_1(BOX_DRAWING_HORIZ_VERT);
        box_drawing_right_down[0] = UTF_8_BYTE_1(BOX_DRAWING_RIGHT_DOWN);
        box_drawing_right_up[0]   = UTF_8_BYTE_1(BOX_DRAWING_RIGHT_UP);
        box_drawing_horiz_down[0] = UTF_8_BYTE_1(BOX_DRAWING_HORIZ_DOWN);
        box_drawing_horiz_up[0]   = UTF_8_BYTE_1(BOX_DRAWING_HORIZ_UP);
        box_drawing_vertical[1]   = UTF_8_BYTE_2(BOX_DRAWING_VERTICAL);
        box_drawing_horizontal[1] = UTF_8_BYTE_2(BOX_DRAWING_HORIZONTAL);
        box_drawing_vert_horiz[1] = UTF_8_BYTE_2(BOX_DRAWING_VERT_HORIZ);
        box_drawing_horiz_vert[1] = UTF_8_BYTE_2(BOX_DRAWING_HORIZ_VERT);
        box_drawing_right_down[1] = UTF_8_BYTE_2(BOX_DRAWING_RIGHT_DOWN);
        box_drawing_right_up[1]   = UTF_8_BYTE_2(BOX_DRAWING_RIGHT_UP);
        box_drawing_horiz_down[1] = UTF_8_BYTE_2(BOX_DRAWING_HORIZ_DOWN);
        box_drawing_horiz_up[1]   = UTF_8_BYTE_2(BOX_DRAWING_HORIZ_UP);
        box_drawing_vertical[2]   = UTF_8_BYTE_3(BOX_DRAWING_VERTICAL);
        box_drawing_horizontal[2] = UTF_8_BYTE_3(BOX_DRAWING_HORIZONTAL);
        box_drawing_vert_horiz[2] = UTF_8_BYTE_3(BOX_DRAWING_VERT_HORIZ);
        box_drawing_horiz_vert[2] = UTF_8_BYTE_3(BOX_DRAWING_HORIZ_VERT);
        box_drawing_right_down[2] = UTF_8_BYTE_3(BOX_DRAWING_RIGHT_DOWN);
        box_drawing_right_up[2]   = UTF_8_BYTE_3(BOX_DRAWING_RIGHT_UP);
        box_drawing_horiz_down[2] = UTF_8_BYTE_3(BOX_DRAWING_HORIZ_DOWN);
        box_drawing_horiz_up[2]   = UTF_8_BYTE_3(BOX_DRAWING_HORIZ_UP);
        box_drawing_vertical[3]   = '\0';
        box_drawing_horizontal[3] = '\0';
        box_drawing_vert_horiz[3] = '\0';
        box_drawing_horiz_vert[3] = '\0';
        box_drawing_right_down[3] = '\0';
        box_drawing_right_up[3]   = '\0';
        box_drawing_horiz_down[3] = '\0';
        box_drawing_horiz_up[3]   = '\0';
    }
}
