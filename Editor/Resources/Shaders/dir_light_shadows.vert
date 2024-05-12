#version 460

layout (location = 0) in vec3 a_coords;

uniform mat4 u_Transform;


void main(){
	gl_Position = u_Transform * vec4(a_coords, 1.0);
}