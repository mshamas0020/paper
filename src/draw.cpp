// draw.cpp

#include "draw.hpp"

#include "composite.h"
#include "fonts.h"
#include "images.h"
#include "ttf_render.h"
#include "epd/epd4in2_V2.h"



static constexpr uint8_t    DARK_GRAY = 0x80;
static constexpr int        DISPLAY_W = 400;
static constexpr int        DISPLAY_H = 300;

namespace paper {

int draw(const DrawInfo& info)
{
    Serial.printf("DRAW    > %s / %s\n", info.time_str, info.date_str);
    Serial.print("DRAW    > Rendering... ");

    const int scratch_bytes = 64 * 1024;
    const int display_bytes = DISPLAY_W * DISPLAY_H / (info.enable_2bpp ? 4 : 8);
    uint32_t start = millis();

    uint8_t* buf_8bpp = (uint8_t*) calloc(scratch_bytes, 1);
    uint8_t* display_Xbpp = (uint8_t*) calloc(display_bytes, 1);

    {
        // time string

        const int x = DISPLAY_W / 2;
        const int y = 50;
        const int w = 320;
        const int h = 160;

        struct TTF_t ttf;
        TTF_init(&ttf, notoserif_regular_ttf);
        TTF_set_canvas(&ttf, buf_8bpp, w, h);
        TTF_render_centered(&ttf, info.time_str, w / 2, 0, 144);

        if (info.enable_2bpp)
            composite_8bpp_to_2bpp(display_Xbpp, x - w / 2, y, w, h, DISPLAY_W, buf_8bpp);
        else
            composite_8bpp_to_1bpp(display_Xbpp, x - w / 2, y, w, h, DISPLAY_W, buf_8bpp);
    }

    {
        // date string

        const int x = DISPLAY_W / 2;
        const int y = 200;
        const int w = 320;
        const int h = 100;
        
        memset(buf_8bpp, 0, scratch_bytes);

        struct TTF_t ttf;
        TTF_init(&ttf, notoserif_bold_ttf);
        TTF_set_canvas(&ttf, buf_8bpp, w, h);
        TTF_render_centered(&ttf, info.date_str, w / 2, 0, 60);

        if (info.enable_2bpp)
            composite_8bpp_to_2bpp_level(display_Xbpp, x - w / 2, y, w, h, DISPLAY_W, buf_8bpp, DARK_GRAY);
        else
            composite_8bpp_to_1bpp(display_Xbpp, x - w / 2, y, w, h, DISPLAY_W, buf_8bpp);
    }

    if (info.network_error) {
        // network error icon

        const int x = 349;
        const int y = 40;
        const int w = img_network_error.w;
        const int h = img_network_error.h;

        if (info.enable_2bpp)
            composite_8bpp_to_2bpp_level(display_Xbpp, x, y, w, h, DISPLAY_W, img_network_error.data_8bpp, DARK_GRAY);
        else
            composite_8bpp_to_1bpp(display_Xbpp, x, y, w, h, DISPLAY_W, img_network_error.data_8bpp);
    }

    // invert colors
    for (int i = 0; i < display_bytes; i += 4) {
        uint32_t* x = (uint32_t*) &display_Xbpp[i];
        *x = ~(*x);
    }
    
    Serial.printf("done (%d ms).\n", (int) (millis() - start));

    Serial.print("DRAW    > Sending to display... ");
    start = millis();

    Epd epd;
    if (info.enable_2bpp) {
        epd.Init_4Gray();
        epd.Set_4GrayDisplay(display_Xbpp, 0, 0, DISPLAY_W, DISPLAY_H);
    } else {
        epd.Init_Fast(Seconds_1S);
        epd.Display_Fast(display_Xbpp);
    }

    Serial.printf("done (%d ms).\n", (int) (millis() - start));

    //delay(5000);

    epd.Sleep();
    free(buf_8bpp);
    free(display_Xbpp);

    return 0;
}

} // namespace paper