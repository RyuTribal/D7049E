#version 460

in vec3 worldSpacePosition;
in vec3 normal;
in vec2 texCoords;
in vec3 cameraPosition;
in flat int v_EntityID;

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

struct DirectionalLightInfo {
    vec3 padding;
    float intensity;
    vec4 color;
    vec4 direction;
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

// Texture samplers
layout(binding = 0) uniform sampler2D u_AlbedoTexture;
layout(binding = 1) uniform sampler2D u_NormalTexture;
layout(binding = 2) uniform sampler2D u_RoughnessTexture;
layout(binding = 3) uniform sampler2D u_MetalnessTexture;
layout(binding = 4) uniform sampler2D u_SpecularTexture;
layout(binding = 5) uniform sampler2D u_EmissiveTexture;


layout(binding = 2, std430) readonly buffer LightBuffer {
    PointLightInfo data[];
} lightBuffer;

layout(binding = 0, std430) readonly buffer DirectionalLightsBuffer {
    DirectionalLightInfo data[];
} directionalLightsBuffer;

layout(binding = 1, std430) readonly buffer VisibleLightIndicesBuffer {
    VisibleIndex data[];
} visibleLightIndicesBuffer;

out vec4 fragColor;
layout(location = 1) out int o_EntityID;

uniform int numberOfTilesX;
uniform int u_NumDirectionalLights;

const float PI = 3.14159265359;

vec3 CalcPointLight(PointLightInfo light, vec3 N, vec3 V, vec3 F0, float roughness, float metalness, vec3 albedo, vec3 P);
vec3 CalcDirectionalLight(DirectionalLightInfo light, vec3 N, vec3 V, vec3 F0, float roughness, float metalness, vec3 albedo);

float DistributionGGX(float NdotH, float roughness);
float GeometrySmith(float NdotV, float NdotL, float roughness);
vec3 FresnelSchlick(float HdotV, vec3 BaseReflectivity);

void main() {

    
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition - worldSpacePosition);
    vec3 albedo = texture(u_AlbedoTexture, texCoords).rgb;
    float roughness = texture(u_RoughnessTexture, texCoords).r;
    float metalness = texture(u_MetalnessTexture, texCoords).r;
    vec3 specular = texture(u_SpecularTexture, texCoords).rgb;
    vec3 emissive = texture(u_EmissiveTexture, texCoords).rgb;
    vec3 lighting = emissive; 

    vec3 BaseReflectivity = mix(vec3(0.04), albedo, metalness);
    vec3 Lo = vec3(0.0);

    ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = tileID.y * numberOfTilesX + tileID.x;
    uint offset = index * 1024;

    for (uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset+i].index != -1; i++) {
        uint lightIndex = visibleLightIndicesBuffer.data[offset+i].index;
        PointLightInfo light = lightBuffer.data[lightIndex];
        Lo += CalcPointLight(light, N, V, BaseReflectivity, roughness, metalness, albedo, worldSpacePosition);
    }

    for (uint i = 0; i < u_NumDirectionalLights; i++) {
        DirectionalLightInfo light = directionalLightsBuffer.data[i];
        Lo += CalcDirectionalLight(light, N, V, BaseReflectivity, roughness, metalness, albedo);
    }

    vec3 ambient = vec3(0.03) * albedo;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));

    color = pow(color, vec3(1.0/2.2));

    fragColor = vec4(color, 1.0);
    o_EntityID = v_EntityID;
}


vec3 CalcPointLight(PointLightInfo light, vec3 N, vec3 V, vec3 F0, float roughness, float metalness, vec3 albedo, vec3 P) {
    vec3 L = normalize(light.position.xyz - P);
    vec3 H = normalize(V + L);
    float distance = length(light.position.xyz - P);
    float attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * distance + light.quadraticAttenuation * distance * distance);
    vec3 radiance = light.color.rgb * light.intensity * attenuation;

    float NDF = DistributionGGX(max(dot(N, H), 0.0), roughness);
    float G = GeometrySmith(max(dot(N, V), 0.0), max(dot(N, L), 0.0), roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    float NdotL = max(dot(N, L), 0.0);

    vec3 diffuse = (albedo / PI) * kD * NdotL;
    vec3 specular = radiance * (NDF * G * F / (4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001));

    return diffuse + specular;
}

vec3 CalcDirectionalLight(DirectionalLightInfo light, vec3 N, vec3 V, vec3 F0, float roughness, float metalness, vec3 albedo) {
    vec3 L = normalize(-light.direction.xyz);
    vec3 H = normalize(V + L);
    vec3 radiance = light.color.rgb * light.intensity;

    float NDF = DistributionGGX(max(dot(N, H), 0.0), roughness);
    float G = GeometrySmith(max(dot(N, V), 0.0), max(dot(N, L), 0.0), roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    float NdotL = max(dot(N, L), 0.0);

    vec3 diffuse = (albedo / PI) * kD * NdotL;
    vec3 specular = radiance * (NDF * G * F / (4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001));

    return diffuse + specular;
}


// PBR calculations

float DistributionGGX(float NdotH, float roughness){
    float a = roughness * roughness;
    float a2 = a*a;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / max(denom, 0.0000001);
}

float GeometrySmith(float NdotV, float NdotL, float roughness){
    float r = roughness + 1.0;
    float k = (r*r)/8.0;
    float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
    float ggx2 = NdotL / (NdotL * (1.0 - k) + k);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float HdotV, vec3 BaseReflectivity){
    return BaseReflectivity + (1.0 - BaseReflectivity) * pow(1.0 - HdotV, 5.0);
}