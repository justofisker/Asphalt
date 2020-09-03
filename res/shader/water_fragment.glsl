#version 330 core

uniform vec4 water_color;
uniform sampler2D screen_tex;

in vec2 UV;

out vec4 color;

void main()
{
    color = texture(screen_tex, UV) * water_color;
}