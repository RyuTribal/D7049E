#version 460

layout (location = 0) in vec3 a_position;

out vec3 localPos;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    localPos = a_position;  
    gl_Position =  u_Projection * u_View * vec4(localPos, 1.0);
}