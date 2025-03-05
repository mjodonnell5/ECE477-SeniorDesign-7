#ifndef FONT_H
#define FONT_H

#include <stdint.h>

struct font {
    const uint8_t* font;
    uint8_t height;
    uint8_t width;
};

extern struct font small_font;
extern struct font large_font;

#endif /* FONT_H */
