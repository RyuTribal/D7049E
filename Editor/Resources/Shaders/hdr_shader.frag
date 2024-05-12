#version 460 core

in vec2 TextureCoordinates;

layout(binding = 0) uniform sampler2D hdrBuffer;

// Controls exposure level of image
uniform float exposure;

out vec4 fragColor;

// Uses Filmic tonemapping (thanks chat gpt)

vec3 FilmicToneMapping(vec3 color) {
    color = max(vec3(0.0), color - 0.004);
    color = (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
}

void main() {
	vec3 color = texture(hdrBuffer, TextureCoordinates).rgb;
	vec3 result = vec3(1.0) - exp(-color * exposure);

	// Minor gamma correction. Need to expand on it
	FilmicToneMapping(result);
	fragColor = vec4(result, 1.0);
}
