#version 460

layout (location = 0) in vec3 a_coords;
layout(location = 1) in vec4 a_colors;
layout(location = 2) in vec2 a_texture_coords;

uniform mat4 u_CameraMatrix;
uniform mat4 u_Transform;

out float cameraDistance;
out vec2 texCoords;

void main(){
	texCoords = a_texture_coords;
	gl_Position = u_CameraMatrix * u_Transform * vec4(a_coords, 1.0);
}