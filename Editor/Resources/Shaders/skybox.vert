#version 460

layout (location = 0) in vec3 a_position;

uniform mat4 u_CameraProjection;
uniform mat4 u_CameraView;

out vec3 local_pos;

void main()
{
	local_pos = a_position;

	mat4 rotView = mat4(mat3(u_CameraView));
	vec4 clipPos = u_CameraProjection * rotView * vec4(local_pos, 1.0);

	gl_Position = clipPos.xyww;
}