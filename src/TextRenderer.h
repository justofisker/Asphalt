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

void setup_textrenderer();

void begin_create_font();
Font *create_font(char *font_location, unsigned int size);
void end_create_font();

void free_font(Font *font);

void render_text(char *text, float x, float y, float color[4], Font *font);

#endif // TEXTRENDERER_H
