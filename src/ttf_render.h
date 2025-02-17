// ttf_render.h

#ifndef __PAPER_TTF_RENDER_H__
#define __PAPER_TTF_RENDER_H__

#include "ext/stb_truetype.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



struct TTF_t
{
    stbtt_fontinfo  stbtt;
    uint8_t*        canvas;
    int             canvas_w;
    int             canvas_h;
    int             ascent;
};

int TTF_init(struct TTF_t* state, const uint8_t* ttf);

void TTF_set_canvas(struct TTF_t* state, uint8_t* buf, int w, int h);

int TTF_render(struct TTF_t* state, const char* str, float x, float y, float line_height);
int TTF_render_width(struct TTF_t* state, const char* str, float line_height);
int TTF_render_centered(struct TTF_t* state, const char* str, float x, float y, float line_height);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __PAPER_TTF_RENDER_H__