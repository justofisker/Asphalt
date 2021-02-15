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

Sprite* Sprite_CreateSprite(Texture *sprite);
void Sprite_DrawSprite(Sprite *sprite);

#endif // SPRITE_H
