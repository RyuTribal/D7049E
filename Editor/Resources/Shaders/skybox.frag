#version 460
out vec4 fragColor;

in vec3 local_pos;

uniform samplerCube u_EnvironmentMap;
uniform float u_Brightness;

void main(){
	vec3 envColor = texture(u_EnvironmentMap, local_pos).rgb * u_Brightness;
	
	// tone mapping is handled in hdr shader

	fragColor = vec4(envColor, 1.0);
}

