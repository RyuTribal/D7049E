#version 460

layout (location = 0) in vec3 a_coords;
layout(location = 1) in vec4 a_colors;

uniform mat4 u_CameraMatrix;
uniform mat4 u_Transform;


void main(){

	gl_Position = u_CameraMatrix * u_Transform * vec4(a_coords, 1.0);

	gl_PointSize = 5.f;
}