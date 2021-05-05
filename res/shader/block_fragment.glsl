#version 330 core

out vec4 color;

in vec2 UV;
flat in int light;

uniform sampler2D u_Texture;

void main()
{
    vec4 tint;
    switch(light)
    {
        case 0:
            tint = vec4(0.8, 0.8, 0.85, 1.0);
            break;
        case 1:
            tint = vec4(0.6, 0.6, 0.65, 1.0);
            break;
        case 2:
            tint = vec4(0.4, 0.4, 0.45, 1.0); 
            break;
        case 3:
        default:
            tint = vec4(1.0, 1.0, 1.0, 1.0);
            break;
    }

    color = texture(u_Texture, UV) * tint;
}