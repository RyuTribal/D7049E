#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 a_texture_coords;

out vec2 TextureCoordinates;

void main() {
	gl_Position = vec4(position, 1.0);
	TextureCoordinates = a_texture_coords;
}