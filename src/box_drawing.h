#ifndef BOX_DRAWING_H
#define BOX_DRAWING_H

extern char *box_drawing_vertical;
extern char *box_drawing_horizontal;
extern char *box_drawing_vert_horiz; /* in ASCII, vertical */
extern char *box_drawing_horiz_vert; /* in ASCII, horizontal */
extern char *box_drawing_right_down;
extern char *box_drawing_right_up;

#define BOX_DRAWING_VERTICAL   0x2502
#define BOX_DRAWING_HORIZONTAL 0x2500
#define BOX_DRAWING_RIGHT_DOWN 0x250c
#define BOX_DRAWING_RIGHT_UP   0x2514
#define BOX_DRAWING_VERT_HORIZ 0x253c
#define BOX_DRAWING_HORIZ_VERT 0x253c

void init_box_drawing(bool);

#endif
