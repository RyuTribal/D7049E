#version 460
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texture_coords;

out vec2 TexCoords;

void main()
{
    TexCoords = a_texture_coords;
	gl_Position = vec4(a_position, 1.0);
}