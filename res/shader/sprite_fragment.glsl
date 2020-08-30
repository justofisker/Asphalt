#version 460

in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
    FragColor = texture(u_Texture, TexCoord);
}
