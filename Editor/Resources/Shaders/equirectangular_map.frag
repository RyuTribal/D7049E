#version 460

out vec4 fragColor;
in vec3 localPos;

uniform sampler2D u_EnvironmentMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 ToneMapReinhard(vec3 color) {
    return color / (color + vec3(1.0));
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(localPos)); // make sure to normalize localPos
    vec3 color = texture(u_EnvironmentMap, uv).rgb;

    //color = ToneMapReinhard(color);
    
    fragColor = vec4(color, 1.0);
}