#version 460

in vec3 worldSpacePosition;
in vec4 vertex_color;
in vec3 normal;
in vec2 texCoords;
in vec3 cameraPosition;
in mat3 TBN;

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

struct VisibleIndex {
    int index;
};

// Texture samplers

layout(binding = 1) uniform sampler2D u_NormalTexture;
layout(binding = 2) uniform sampler2D u_RoughnessTexture;
layout(binding = 3) uniform sampler2D u_MetalnessTexture;
layout(binding = 5) uniform sampler2D u_AlbedoTexture;


struct Material
{
	vec3 AlbedoColor;
	float Metalness;
	float Roughness;
	float Emission;
	bool UseNormalMap;
};

uniform Material u_MaterialUniforms;


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

uniform int numberOfTilesX;
uniform int u_NumDirectionalLights;
uniform int u_UsesHeightMap;
uniform int u_UsesNormalMap;

const float PI = 3.14159265359;

vec3 CalcPointLight(PointLightInfo light, vec3 N, vec3 V, vec3 albedo, float roughness, float metalness, vec3 P);
vec3 CalcDirectionalLight(DirectionalLightInfo light, vec3 N, vec3 V, vec3 albedo, float roughness, float metalness);

float DistributionGGX(float NdotH, float roughness);
float GeometrySmith(float NdotV, float NdotL, float roughness);
vec3 FresnelSchlick(float HdotV, vec3 BaseReflectivity);

vec3 sRGBToLinear(vec3 sRGB) {
    return vec3(
    sRGB.r <= 0.04045 ? sRGB.r / 12.92 : pow((sRGB.r + 0.055) / 1.055, 2.4),
    sRGB.g <= 0.04045 ? sRGB.g / 12.92 : pow((sRGB.g + 0.055) / 1.055, 2.4),
    sRGB.b <= 0.04045 ? sRGB.b / 12.92 : pow((sRGB.b + 0.055) / 1.055, 2.4)
    );
}

void main() {
    vec3 V = normalize(cameraPosition - worldSpacePosition);
    vec3 N;
    if(u_MaterialUniforms.UseNormalMap){
        vec3 normalMap = texture(u_NormalTexture, texCoords).xyz;
        normalMap = normalMap * 2.0 - 1.0;
        N = normalize(TBN * normalMap);
    }
    else{
       N = normalize(TBN * normal); 
    }
    vec3 albedo = texture(u_AlbedoTexture, texCoords).rgb * u_MaterialUniforms.AlbedoColor;
    float roughness = texture(u_RoughnessTexture, texCoords).r * u_MaterialUniforms.Roughness;
    float metalness = texture(u_MetalnessTexture, texCoords).r * u_MaterialUniforms.Metalness;

    vec3 BaseReflectivity = mix(vec3(0.04), albedo, metalness);
    vec3 Lo = vec3(0.0);

    ivec2 location = ivec2(gl_FragCoord.xy);
	ivec2 tileID = location / ivec2(16, 16);
	uint index = tileID.y * numberOfTilesX + tileID.x;
    uint offset = index * 1024;

    for (uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset+i].index != -1; i++) {
        uint lightIndex = visibleLightIndicesBuffer.data[offset+i].index;
        PointLightInfo light = lightBuffer.data[lightIndex];
        Lo += CalcPointLight(light, N, V, albedo, roughness, metalness, worldSpacePosition);
    }

    for (uint i = 0; i < u_NumDirectionalLights; i++) {
        DirectionalLightInfo light = directionalLightsBuffer.data[i];
        Lo += CalcDirectionalLight(light, N, V, albedo, roughness, metalness);
    }

    vec3 ambient = vec3(0.03) * albedo;
    vec3 color = ambient + Lo;

    fragColor = vec4(color, 1.0);
}


vec3 CalcPointLight(PointLightInfo light, vec3 N, vec3 V, vec3 albedo, float roughness, float metalness, vec3 P) {
    vec3 L = normalize(light.position.xyz - P);
    vec3 H = normalize(V + L);
    float distance = length(light.position.xyz - P);
    float attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * distance + light.quadraticAttenuation * distance * distance);
    vec3 radiance = light.color.rgb * light.intensity * attenuation;

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float NdotH = max(dot(N, H), 0.0);

    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    float D = DistributionGGX(NdotH, roughness);

    float G = GeometrySmith(NdotV, NdotL, roughness);

    vec3 specular = (F * D * G) / max(4.0 * NdotL * NdotV, 0.001);

    vec3 kd = vec3(1.0 - max(F.r, max(F.g, F.b))) * (1.0 - metalness);
    vec3 diffuse = kd * albedo / PI;

    return (diffuse + specular) * radiance * NdotL;
}



vec3 CalcDirectionalLight(DirectionalLightInfo light, vec3 N, vec3 V, vec3 albedo, float roughness, float metalness) {
    vec3 L = normalize(-light.direction.xyz);
    vec3 radiance = light.color.rgb * light.intensity;

    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float NdotH = max(dot(N, H), 0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    float D = DistributionGGX(NdotH, roughness);
    float G = GeometrySmith(NdotV, NdotL, roughness);
    vec3 specular = (F * D * G) / max(4.0 * NdotL * NdotV, 0.001);
    vec3 kd = vec3(1.0 - max(F.r, max(F.g, F.b))) * (1.0 - metalness);
    vec3 diffuse = kd * albedo / PI;
    return (diffuse + specular) * radiance * NdotL;
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