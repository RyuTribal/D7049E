#version 460

layout (location = 0) in vec3 a_position;

out vec3 local_pos;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    local_pos = a_position;  
    gl_Position =  u_Projection * u_View * vec4(local_pos, 1.0);
}