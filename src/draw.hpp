// draw.hpp

#ifndef __PAPER_DRAW_HPP__
#define __PAPER_DRAW_HPP__



namespace paper {

struct DrawInfo
{
    bool        enable_2bpp;
    bool        network_error;
    const char* time_str;
    const char* date_str;
};

int draw(const DrawInfo& info);

} // namespace paper



#endif // __PAPER_DRAW_HPP__