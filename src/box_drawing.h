#ifndef BOX_DRAWING_H
#define BOX_DRAWING_H

extern char *box_drawing_vertical;
extern char *box_drawing_horizontal;
extern char *box_drawing_vert_horiz; /* in ASCII, vertical */
extern char *box_drawing_horiz_vert; /* in ASCII, horizontal */
extern char *box_drawing_right_down;
extern char *box_drawing_right_up;
extern char *box_drawing_horiz_up;
extern char *box_drawing_horiz_down;

#define UTF_8_BYTE_1(codepoint) ((unsigned char)((0xe0 | ((codepoint >> 12) & 0x1f))))
#define UTF_8_BYTE_2(codepoint) ((unsigned char)((0x80 | ((codepoint >> 6)  & 0x3f))))
#define UTF_8_BYTE_3(codepoint) ((unsigned char)((0x80 | ((codepoint >> 0)  & 0x3f))))

#define BOX_DRAWING_VERTICAL   0x2502
#define BOX_DRAWING_HORIZONTAL 0x2500
#define BOX_DRAWING_RIGHT_DOWN 0x250c
#define BOX_DRAWING_RIGHT_UP   0x2514
#define BOX_DRAWING_VERT_HORIZ 0x253c
#define BOX_DRAWING_HORIZ_VERT 0x253c
#define BOX_DRAWING_HORIZ_UP   0x2534
#define BOX_DRAWING_HORIZ_DOWN 0x252c

#define BOX_DRAWING_UTF_8_VERTICAL   ""
#define BOX_DRAWING_UTF_8_HORIZONTAL ""
#define BOX_DRAWING_UTF_8_RIGHT_DOWN ""
#define BOX_DRAWING_UTF_8_RIGHT_UP   ""
#define BOX_DRAWING_UTF_8_VERT_HORIZ ""
#define BOX_DRAWING_UTF_8_HORIZ_VERT ""

void init_box_drawing(bool);

#endif
