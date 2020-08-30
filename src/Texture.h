#ifndef TEXTURE_H
#define TEXTURE_H

#include <cglm/cglm.h>

typedef struct _Texture {
    unsigned int texture_id;
    unsigned int width, height;
    unsigned int bpp;
} Texture;

Texture* create_texture(const char* file_path, int texture_min_filter, int texture_mag_filter, int texture_wrap, char mipmap, float lod_bias);
Texture* create_texture_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void bind_texture(Texture* texture, unsigned int slot);

#endif // TEXTURE_H
