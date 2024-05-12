#version 460

layout (location = 0) in vec3 a_coords;
layout (location = 1) in vec4 a_colors;
layout (location = 2) in vec2 a_texture_coords;
layout (location = 3) in vec3 a_normals;
layout (location = 4) in vec3 a_tangent;
layout (location = 5) in vec3 a_bitangent;

uniform mat4 u_CameraView;
uniform mat4 u_CameraProjection;
uniform vec3 u_CameraPos;
uniform mat4 u_Transform;
uniform mat4 u_SunView;
uniform mat4 u_SunProjection;

out vec3 worldSpacePosition;
out vec4 vertex_color;
out vec3 normal;
out mat3 TBN;
out vec2 texCoords;
out vec4 objectColor;
out vec3 cameraPosition;
out vec4 fragLightSpacePosition;


void main() {
    gl_Position = u_CameraProjection * u_CameraView * u_Transform * vec4(a_coords, 1.0);
    worldSpacePosition = vec3(u_Transform * vec4(a_coords, 1.0));

    fragLightSpacePosition = u_SunProjection * u_SunView * vec4(worldSpacePosition, 1.0);

    vec3 N = normalize(mat3(u_Transform) * a_normals);
    vec3 T = normalize(mat3(u_Transform) * a_tangent);
    vec3 B = normalize(mat3(u_Transform) * a_bitangent);
    TBN = mat3(T, B, N);

    normal = N;

    cameraPosition = u_CameraPos;
    texCoords = a_texture_coords;
    vertex_color = a_colors;
}