#version 460

in vec2 texCoords;

uniform sampler2D u_Texture;

layout(location = 0) out vec4 outColor;

void main(){

    outColor = texture(u_Texture, texCoords);
}