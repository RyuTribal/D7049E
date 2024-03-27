#version 460

in vec3 worldSpacePosition;
in vec3 normal;
in vec2 texCoords;
in vec3 cameraPosition;

#define DIFFUSE_RATE 0.5
#define AMBIENT_RATE 0.2

struct PointLightInfo {
    float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
	float intensity;
	vec4 color;
    vec4 position;
};

struct Material {
    float shininess;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material u_Material;

struct VisibleIndex {
    int index;
};

layout(std430, binding = 1) readonly buffer VisibleLightIndicesBuffer {
    VisibleIndex data[];
} visibleLightIndicesBuffer;

layout(std430, binding = 0) readonly buffer LightBuffer {
    PointLightInfo data[];
} lightBuffer;

out vec4 fragColor;

uniform int numberOfTilesX;

vec3 CalcPointLight(PointLightInfo light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {


    ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = tileID.y * numberOfTilesX + tileID.x;

    vec3 normal_normalized = normalize(normal);
    vec3 viewDirection = normalize(cameraPosition - worldSpacePosition);

    vec4 final_color = vec4(0.0, 0.0, 0.0, 1.0);

    uint offset = index * 1024;
    for (uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset+i].index != -1; i++) {
        uint lightIndex = visibleLightIndicesBuffer.data[offset+i].index;
        PointLightInfo light = lightBuffer.data[lightIndex];

        final_color.rgb += CalcPointLight(light, normal_normalized, worldSpacePosition, viewDirection);
    }

    fragColor = final_color;
}


vec3 CalcPointLight(PointLightInfo light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position.xyz - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);

    float distance    = length(light.position.xyz - fragPos);
    float attenuation = light.intensity / (light.constantAttenuation + light.linearAttenuation * distance + 
  			     light.quadraticAttenuation * (distance * distance));    

    vec3 ambient  = light.color.rgb * AMBIENT_RATE * u_Material.ambient;
    vec3 diffuse  = light.color.rgb * DIFFUSE_RATE * diff * u_Material.diffuse;
    vec3 specular = light.color.rgb * spec * u_Material.specular;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}