#version 330 core

uniform sampler2D screen_tex;

in vec2 UV;

out vec4 color;

void main()
{
    color = vec4(vec3(1.0) - texture(screen_tex, UV).rgb, 1.0);
}