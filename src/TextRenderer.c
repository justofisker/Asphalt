#include "TextRenderer.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cglm/cglm.h>
#include <glad/glad.h>
#include "Util.h"
#include "Globals.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

static unsigned int text_shader;
static unsigned int VAO, VB, IB;
static int textcolor_loc, projection_loc;
static FT_Library ft;

typedef struct _Vertex {
    vec2 position;
    vec2 uv;
} Vertex;

typedef unsigned short Index;

void Text_Setup()
{
    text_shader = Util_CompileShader("res/shader/text_vertex.glsl", "res/shader/text_fragment.glsl");

    textcolor_loc = glGetUniformLocation(text_shader, "u_TextColor");
    projection_loc = glGetUniformLocation(text_shader, "u_Projection");

    Index indices[6] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VB);
    glBindBuffer(GL_ARRAY_BUFFER, VB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glGenBuffers(1, &IB);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);   
}

void Text_BeginCreateFont()
{
    if(FT_Init_FreeType(&ft))
    {
        printf("Could not init Freetype Library :<\n");
        return;
    }
}

Font *Text_CreateFont(char *font_location, unsigned int size)
{
    FT_Face face;
    if(FT_New_Face(ft, font_location, 0, &face))
    {
        printf("Failed to load font\n");
        return NULL;
    }

    FT_Set_Pixel_Sizes(face, 0, size);

    Font *font = malloc(sizeof(Font));

    font->size = size;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    unsigned char c;
    for (c = 0; c < 128; c++)
    {
        if(FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            printf("Failed to load Gyph!\n");
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        font->characters[c] = (Character){
            texture,
            {face->glyph->bitmap.width, face->glyph->bitmap.rows},
            {face->glyph->bitmap_left, face->glyph->bitmap_top},
            face->glyph->advance.x
        };
    }

    FT_Done_Face(face);

    return font;
}

void Text_EndCreateFont()
{
    FT_Done_FreeType(ft);
}

void Text_FreeFont(Font *font)
{
    int i;
    for (i = 0; i < 128; i++)
    {
        if(font->characters[i].texture_id)
            glDeleteTextures(1, &font->characters[i].texture_id);
    }
    free(font);
}

TextPos Text_RenderTextBuffer(char *text, float x, float y, float color[4], Font *font)
{
    y = g_height - y - font->size;
    
    glUseProgram(text_shader);
    glUniform4fv(textcolor_loc, 1, color);
    mat4 projection;
    glm_ortho(0.0f, g_width, 0.0f, g_height, 0.0f, 1.0f, projection);
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, projection[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    int c;
    for (c = 0; text[c]; c++)
    {
        if(text[c] >= 128 || text[c] < 0) continue;
        Character *ch = &font->characters[text[c]];

        float xpos = x + ch->bearing[0];
        float ypos = y - (ch->size[1] - ch->bearing[1]);

        float w = ch->size[0];
        float h = ch->size[1];
        Vertex vertices[4] = {
            {{ xpos    , ypos + h}, {0.0f, 0.0f}},            
            {{ xpos    , ypos    }, {0.0f, 1.0f}},
            {{ xpos + w, ypos    }, {1.0f, 1.0f}},
            {{ xpos + w, ypos + h}, {1.0f, 0.0f}}           
        };
        glBindTexture(GL_TEXTURE_2D, ch->texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, VB);
        //printf("%u\n", VB);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        //glBufferData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
        x += (ch->advance >> 6);
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return (TextPos){x, g_height - y };
}

TextPos Text_RenderText(char *format, float x, float y, float color[4], Font *font, ...)
{
    static char buf[1024];
    va_list args;
    va_start (args, format);
    vsprintf_s(buf, sizeof(buf), format, args );
    va_end (args);

    return Text_RenderTextBuffer(buf, x, y, color, font);
}
