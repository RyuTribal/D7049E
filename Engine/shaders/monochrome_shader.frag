#version 460

in vec3 fragColor;
in float cameraDistance;

layout(location = 0) out vec4 outColor;

void main(){
	

    float attenuation = 1.5 / (1.0 + cameraDistance); 
    attenuation = clamp(attenuation, 0.0, 1.0); 

    vec3 attenuatedColor = fragColor * attenuation;

    outColor = vec4(attenuatedColor, 1.0f);
}