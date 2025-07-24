#version 460 core

#define MAX_SAMPLERS 5

in VS_OUT {
    vec3 normal;
    vec3 fragPosition;
    vec2 texCoord;
} fs_in;

uniform uint PBZ_TextureDiffuseLength;
uniform sampler2D PBZ_TextureDiffuse[MAX_SAMPLERS];

uniform uint PBZ_TextureSpecularLength;
uniform sampler2D PBZ_TextureSpecular[MAX_SAMPLERS];

layout(location = 0) out vec3 PBZ_GBuffer0; // position
layout(location = 1) out vec3 PBZ_GBuffer1; // normals
layout(location = 2) out vec4 PBZ_GBuffer2; // albedo+spec

void main() {
    vec3 albedo = vec3(0.0);
    float specular = 0.0;

    for (uint i = 0; i < PBZ_TextureDiffuseLength; i++) {
        albedo += texture(PBZ_TextureDiffuse[i], fs_in.texCoord).rgb;
    }

    if (PBZ_TextureDiffuseLength > 0) {
        albedo /= float(PBZ_TextureDiffuseLength);
    }

    for (uint i = 0; i < PBZ_TextureSpecularLength; ++i) {
        specular += texture(PBZ_TextureSpecular[i], fs_in.texCoord).r;
    }

    if (PBZ_TextureSpecularLength > 0) {
        specular /= float(PBZ_TextureSpecularLength);
    }

    PBZ_GBuffer0 = fs_in.fragPosition;
    PBZ_GBuffer1 = normalize(fs_in.normal);
    PBZ_GBuffer2 = vec4(albedo, specular);
}
