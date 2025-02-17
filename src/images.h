// images.h

#ifndef __PAPER_IMAGES_H__
#define __PAPER_IMAGES_H__



struct Image8bpp
{
    int w;
    int h;
    const unsigned char* data_8bpp;
};

extern const struct Image8bpp img_network_error;



#endif // __PAPER_IMAGES_H__