// composite.c

#include "composite.h"



static uint8_t to_1bpp(uint8_t x)
{
    return (x & 0x80);
}

void composite_8bpp_to_1bpp(uint8_t* out_1bpp, int x, int y, int w, int h,
        int stride, const uint8_t* in_8bpp)
{
    const uint8_t* in = in_8bpp;

    for (int i = 0; i < h; i++) {
        uint8_t* out = out_1bpp + (y + i) * (stride / 8) + x / 8;

        for (int j = 0; j < w; j += 8) {
            for (int bit = 0; bit < 8; bit++)
                *out |= (to_1bpp(*in++)) >> bit;
            *out++;
        }
    }
}



static uint8_t to_2bpp(uint8_t x)
{
    x &= 0xC0;
    x |= (x & 0x80) >> 2;
    x <<= 1;
    return x;
}

void composite_8bpp_to_2bpp(uint8_t* out_2bpp, int x, int y, int w, int h,
        int stride, const uint8_t* in_8bpp)
{
    const uint8_t* in = in_8bpp;

    for (int i = 0; i < h; i++) {
        uint8_t* out = out_2bpp + (y + i) * (stride / 4) + x / 4;

        for (int j = 0; j < w; j += 4) {
            for (int bit = 0; bit < 8; bit += 2)
                *out |= to_2bpp(*in++) >> bit;
            *out++;
        }
    }
}

static uint8_t min_level(uint8_t a, uint8_t b)
{
    return (a < b) ? a : b;
}

void composite_8bpp_to_2bpp_level(uint8_t* out_2bpp, int x, int y, int w, int h,
    int stride, const uint8_t* in_8bpp, uint8_t level)
{
    const uint8_t* in = in_8bpp;

    for (int i = 0; i < h; i++) {
        uint8_t* out = out_2bpp + (y + i) * (stride / 4) + x / 4;

        for (int j = 0; j < w; j += 4) {
            for (int bit = 0; bit < 8; bit += 2)
                *out |= to_2bpp(min_level(*in++, level)) >> bit;
            *out++;
        }
    }
}