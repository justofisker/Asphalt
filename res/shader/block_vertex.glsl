#version 330 core

layout(location = 0) in int data1;
layout(location = 1) in int data2;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 UV;
flat out int light;

vec3 get_position()
{
    return vec3(float(data1 & 0x1F), float((data1 >> 5) & 0x1FF), float((data1 >> 14) & 0x1F));
}

vec2 get_uv()
{
    int texCoordX = (data1 >> 19) & 0xFF;
    int texCoordY = ((data1 >> 27) & 0x3F) | ((data2 & 0x3) << 6);
    return vec2(float(texCoordX) / 128.0, float(texCoordY) / 128.0);
}

int get_light()
{
    return (data2 >> 2) & 0x3;
}

void main()
{
    UV = get_uv();
    light = get_light();
    gl_Position = u_Projection * u_View * u_Model * vec4(get_position(), 1.0);
}
