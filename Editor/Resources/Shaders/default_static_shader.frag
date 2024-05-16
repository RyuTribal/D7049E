#version 460

in vec3 worldSpacePosition;
in vec4 vertex_color;
in vec3 normal;
in vec2 texCoords;
in vec3 cameraPosition;
in mat3 TBN;
in vec4 fragLightSpacePosition;

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
layout(binding = 6) uniform sampler2D u_AOTexture;
layout(binding = 7) uniform sampler2D u_EmissionTexture;
layout(binding = 8) uniform sampler2D u_SpecularTexture;


layout(binding = 10) uniform samplerCube u_IrradianceMap;
layout(binding = 11) uniform samplerCube u_PrefilterMap;
layout(binding = 12) uniform sampler2D u_BrdfLUT;

layout(binding = 13) uniform sampler2DArray u_ShadowMap;

layout (std140) uniform u_LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};
uniform float u_CascadePlaneDistances[16];
uniform int u_CascadeCount;

uniform float u_EnvironmentBrightness;
uniform float u_CameraFarPlane;
uniform mat4 u_CameraView;


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

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

float ShadowCalculation(float bias, vec3 N);

vec3 sRGBToLinear(vec3 sRGB) {
    return vec3(
    sRGB.r <= 0.04045 ? sRGB.r / 12.92 : pow((sRGB.r + 0.055) / 1.055, 2.4),
    sRGB.g <= 0.04045 ? sRGB.g / 12.92 : pow((sRGB.g + 0.055) / 1.055, 2.4),
    sRGB.b <= 0.04045 ? sRGB.b / 12.92 : pow((sRGB.b + 0.055) / 1.055, 2.4)
    );
}

void main() {
    vec3 V = normalize(cameraPosition - worldSpacePosition);
    vec3 N = normal;
    if (u_MaterialUniforms.UseNormalMap) {
        vec3 normalMap = texture(u_NormalTexture, texCoords).xyz;
        normalMap = normalMap * 2.0 - 1.0;
        N = normalize(TBN * normalMap);
    }
    vec3 R = reflect(-V, N); 

    vec3 specular_color = texture(u_SpecularTexture, texCoords).rgb;
    vec3 albedo = texture(u_AlbedoTexture, texCoords).rgb * u_MaterialUniforms.AlbedoColor * specular_color;
    float roughness = texture(u_MetalnessTexture, texCoords).g * u_MaterialUniforms.Roughness;
    float metalness = texture(u_MetalnessTexture, texCoords).b * u_MaterialUniforms.Metalness;
    vec3 ao = texture(u_AOTexture, texCoords).rgb;
    vec3 emission = texture(u_EmissionTexture, texCoords).rgb;

    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 Lo = vec3(0.0);

    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * numberOfTilesX + tileID.x;
    uint offset = index * 1024;

    for (uint i = 0; i < 1024 && visibleLightIndicesBuffer.data[offset + i].index != -1; i++) {
        uint lightIndex = visibleLightIndicesBuffer.data[offset + i].index;
        PointLightInfo light = lightBuffer.data[lightIndex];
        Lo += CalcPointLight(light, N, V, albedo, roughness, metalness, worldSpacePosition);
    }

    for (uint i = 0; i < u_NumDirectionalLights; i++) {
        DirectionalLightInfo light = directionalLightsBuffer.data[i];
        Lo += CalcDirectionalLight(light, N, V, albedo, roughness, metalness);
    }

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metalness;	  
    vec3 irradiance = texture(u_IrradianceMap, N).rgb;

//    float shadow_bias = max(0.05 * (1.0 - dot(N, directionalLightsBuffer.data[0].direction.rgb)), 0.005);  
//    float shadow = ShadowCalculation(shadow_bias, N);
    vec3 diffuse = irradiance * albedo * u_EnvironmentBrightness;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(u_BrdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = ambient + Lo + emission;
    color = color;

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

    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 specular = (NDF * G * F) / max(4.0 * NdotL * NdotV, 0.001);

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metalness;

    vec3 diffuse = kD * albedo / PI;

    return (diffuse + specular) * radiance * NdotL;
}

vec3 CalcDirectionalLight(DirectionalLightInfo light, vec3 N, vec3 V, vec3 albedo, float roughness, float metalness) {
    vec3 L = normalize(-light.direction.xyz);
    vec3 radiance = light.color.rgb * light.intensity;

    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);

    vec3 F0 = mix(vec3(0.04), albedo, metalness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 specular = (NDF * G * F) / max(4.0 * NdotL * NdotV, 0.001);

    vec3 kD = vec3(1.0) - F;
    kD *= 1.0 - metalness;

    vec3 diffuse = kD * albedo / PI;

    return (diffuse + specular) * radiance * NdotL;
}



// PBR calculations

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

// Shadows

float ShadowCalculation(float bias, vec3 N)
{
    vec4 fragPosViewSpace = u_CameraView * vec4(worldSpacePosition, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    int layer = -1;
    for (int i = 0; i < u_CascadeCount; ++i)
    {
        if (depthValue < u_CascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = u_CascadeCount;
    }

    vec4 fragPosLightSpace = lightSpaceMatrices[layer] * vec4(worldSpacePosition, 1.0);
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    // calculate bias (based on depth map resolution and slope)
    const float biasModifier = 0.5f;
    if (layer == u_CascadeCount)
    {
        bias *= 1 / (u_CameraFarPlane * biasModifier);
    }
    else
    {
        bias *= 1 / (u_CascadePlaneDistances[layer] * biasModifier);
    }

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(u_ShadowMap, 0));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(u_ShadowMap, vec3(projCoords.xy + vec2(x, y) * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
        
    return shadow;
}