#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

typedef struct _Character {
    unsigned int texture_id;
    int size[2];
    int bearing[2];
    unsigned int advance;
} Character;

typedef struct _Font {
    Character characters[128];
    unsigned int size;
} Font;

void Text_Setup();

void Text_BeginCreateFont();
Font *Text_CreateFont(char *font_location, unsigned int size);
void Text_EndCreateFont();

void Text_FreeFont(Font *font);

void Text_RenderText(char *text, float x, float y, float color[4], Font *font);

#endif // TEXTRENDERER_H
