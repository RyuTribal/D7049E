#version 460

layout(location = 0) in vec3 a_position;
uniform mat4 u_CameraView;
uniform mat4 u_CameraProjection;

void main()
{
    gl_Position = u_CameraProjection * u_CameraView * vec4(a_position, 1.0);
}