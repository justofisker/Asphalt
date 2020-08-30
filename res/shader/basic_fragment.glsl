#version 330 core

out vec4 color;

in vec2 UV;
in vec3 Normal;

uniform sampler2D u_Texture;

void main()
{
    vec4 tint;
    if(Normal.x != 0.0)
        tint = vec4(0.8, 0.8, 0.8, 1.0);
    else if (Normal.z != 0.0)
        tint = vec4(0.6, 0.6, 0.6, 1.0);
    else
        tint = vec4(1.0, 1.0, 1.0, 1.0);

    color = texture(u_Texture, UV) * tint;
}