#include "box_drawing.h"

void init_box_drawing(bool force_ascii) {
    box_drawing_vertical = "|";
    box_drawing_horizontal = "-";
    box_drawing_vert_horiz = "|";
    box_drawing_horiz_vert = "-";
    box_drawing_right_down = ".";
    box_drawing_right_up = "'";
}
