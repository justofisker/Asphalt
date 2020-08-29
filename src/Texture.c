#include "Texture.h"
#include <glad/glad.h>
#include <stdlib.h>
#include <stb/stb_image.h>

Texture* create_texture(const char* file_path, int texture_min_filter, int texture_mag_filter, int texture_wrap)
{
    Texture *texture = malloc(sizeof(Texture));

    unsigned char *image_buffer;

    stbi_set_flip_vertically_on_load(1);
    image_buffer = stbi_load(file_path, &texture->width, &texture->height, &texture->bpp, 4);

    glGenTextures(1, &texture->texture_id);
    glBindTexture(GL_TEXTURE_2D, texture->texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_buffer);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    if(image_buffer)
        stbi_image_free(image_buffer);

    return texture;
}

Texture* create_texture_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    Texture *texture = malloc(sizeof(Texture));

    unsigned char image_buffer[4] = {
        r, g, b, a
    };

    glGenTextures(1, &texture->texture_id);
    glBindTexture(GL_TEXTURE_2D, texture->texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_buffer);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void bind_texture(Texture* texture, unsigned int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture->texture_id);
}
