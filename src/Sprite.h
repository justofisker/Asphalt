#ifndef SPRITE_H
#define SPRITE_H

typedef struct _Texture Texture;
typedef float vec2[2];

typedef struct _Sprite {
    Texture *texture;
    vec2 position;
    vec2 scale;
    float rotation;
} Sprite;

Sprite* create_sprite(Texture *sprite);
void draw_sprite(Sprite *sprite);

#endif // SPRITE_H
