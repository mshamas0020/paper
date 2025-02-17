// ttf_render.c

#include "ttf_render.h"

#define STB_TRUETYPE_IMPLEMENTATION 
#include "ext/stb_truetype.h" /* http://nothings.org/stb/stb_truetype.h */

#include <assert.h>

#include <stdio.h>



int TTF_init(struct TTF_t* state, const uint8_t* ttf)
{
    assert(state);

    stbtt_fontinfo* stbtt = &state->stbtt;

    // init stbtt font
    if (!stbtt_InitFont(stbtt, ttf, 0))
        return -1;

    int descent, line_gap;
    stbtt_GetFontVMetrics(stbtt, &state->ascent, &descent, &line_gap);

    return 0;
}

void TTF_set_canvas(struct TTF_t* state, uint8_t* buf, int w, int h)
{
    assert(state);
    assert(buf);
    assert(w);
    assert(h);

    state->canvas = buf;
    state->canvas_w = w;
    state->canvas_h = h;
}

static int ifloor(float x)
{
    return (int) floor(x);
}

static int iceil(float x)
{
    return (int) ceil(x);
}

static float ffrac(float x)
{
    return x - floor(x);
}

static int draw_vline(struct TTF_t* state, int x)
{
    for (int i = 0; i < state->canvas_h; i++)
        state->canvas[i * state->canvas_w + x] = 0x0F;
}

int TTF_render_width(struct TTF_t* state, const char* str, float line_height)
{
    assert(state);

    stbtt_fontinfo* stbtt = &state->stbtt;
    uint8_t* canvas = state->canvas;
    const int canvas_w = state->canvas_w;

    const float scale = stbtt_ScaleForPixelHeight(stbtt, line_height);
    const float ascent = (float) state->ascent * scale;
    int glyph = stbtt_FindGlyphIndex(stbtt, (int) str[0]);

    float size = 0.f;

    for (int i = 0; str[i] != '\0' ; i++) {
        int advance, _;
        stbtt_GetGlyphHMetrics(stbtt, glyph, &advance, &_);

        // get next glyph
        const int next_codepoint = (int) str[i + 1];
        const int next_glyph = stbtt_FindGlyphIndex(stbtt, next_codepoint);
        const int kern = stbtt_GetGlyphKernAdvance(stbtt, glyph, next_glyph);
        size += (advance + kern) * scale;
        glyph = next_glyph;
    }

    return iceil(size);
}

int TTF_render(struct TTF_t* state, const char* str, float x, float y, float line_height)
{
    assert(state);

    stbtt_fontinfo* stbtt = &state->stbtt;
    uint8_t* canvas = state->canvas;
    const int canvas_w = state->canvas_w;

    const float scale = stbtt_ScaleForPixelHeight(stbtt, line_height);
    const float ascent = (float) state->ascent * scale;
    int glyph = stbtt_FindGlyphIndex(stbtt, (int) str[0]);

    float pos_x = x;
    float size = 0.f;

    const float pos_y = y + ascent;
    const int pos_y_floor = ifloor(pos_y);
    const int pos_y_frac = ffrac(pos_y);

    for (int i = 0; str[i] != '\0'; i++) {

        // get horizontal metrics
        int advance;
        int left_side_bearing;
        stbtt_GetGlyphHMetrics(stbtt, glyph, &advance, &left_side_bearing);

        // calc x draw position
        const float draw_x = pos_x + left_side_bearing * scale;
        const int draw_x_floor = ifloor(draw_x);
        const float draw_x_frac = ffrac(draw_x);

        // get bounding box coordinates, relative to current position 
        int glyph_x1, glyph_y1, glyph_x2, glyph_y2;
        stbtt_GetGlyphBitmapBoxSubpixel(stbtt, glyph, scale, scale, draw_x_frac,
                pos_y_frac, &glyph_x1, &glyph_y1, &glyph_x2, &glyph_y2);
        const int glyph_w = glyph_x2 - glyph_x1;
        const int glyph_h = glyph_y2 - glyph_y1;

        const int draw_y_floor = pos_y_floor + glyph_y1;
        const float draw_y_frac = pos_y_frac;
        
        // render glyph
        const int offset = draw_y_floor * canvas_w + draw_x_floor;
        stbtt_MakeGlyphBitmapSubpixel(stbtt, &canvas[offset], glyph_w, glyph_h,
                canvas_w, scale, scale, draw_x_frac, draw_y_frac, glyph);

        // get next glyph
        const int next_codepoint = (int) str[i + 1];
        const int next_glyph = stbtt_FindGlyphIndex(stbtt, next_codepoint);

        // get kerning
        const int kern = stbtt_GetGlyphKernAdvance(stbtt, glyph, next_glyph);

        // advance x coordinate
        const float advance_x = (advance + kern) * scale;
        pos_x += advance_x;
        size += advance_x;

        glyph = next_glyph;
    }

    return iceil(size);
}

int TTF_render_centered(struct TTF_t* state, const char* str, float x, float y, float line_height)
{
    const int width = TTF_render_width(state, str, line_height);
    TTF_render(state, str, x - width / 2.f, y, line_height);
}