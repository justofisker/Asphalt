#version 460 core

out vec4 color;

in vec2 UV;

uniform sampler2D u_Texture;

void main()
{
    color = texture(u_Texture, UV);
}