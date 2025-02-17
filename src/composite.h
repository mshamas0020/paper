// composite.h

#ifndef __PAPER_COMPOSITE_H__
#define __PAPER_COMPOSITE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



void composite_8bpp_to_1bpp(uint8_t* out_1bpp, int x, int y, int w, int h,
        int stride, const uint8_t* in_8bpp);

void composite_8bpp_to_2bpp(uint8_t* out_2bpp, int x, int y, int w, int h,
        int stride, const uint8_t* in_8bpp);

void composite_8bpp_to_2bpp_level(uint8_t* out_2bpp, int x, int y, int w, int h,
        int stride, const uint8_t* in_8bpp, uint8_t level);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PAPER_COMPOSITE_H__