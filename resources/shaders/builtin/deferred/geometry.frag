#version 460 core

#define MAX_SAMPLERS 5

in VS_OUT {
    mat3 TBN;
    vec3 normal;
    vec3 fragPosition;
    vec2 texCoord;
} fs_in;

uniform uint PBZ_TextureDiffuseLength;
uniform sampler2D PBZ_TextureDiffuse[MAX_SAMPLERS];

uniform uint PBZ_TextureSpecularLength;
uniform sampler2D PBZ_TextureSpecular[MAX_SAMPLERS];

uniform uint PBZ_TextureHeightLength;
uniform sampler2D PBZ_TextureHeight[MAX_SAMPLERS];

layout(location = 0) out vec4 PBZ_GBuffer0; // position
layout(location = 1) out vec4 PBZ_GBuffer1; // normals
layout(location = 2) out vec4 PBZ_GBuffer2; // albedo+spec

void main() {
    vec3 albedo = vec3(0.0f);

    for (uint i = 0; i < PBZ_TextureDiffuseLength; i++) {
        albedo += texture(PBZ_TextureDiffuse[i], fs_in.texCoord).rgb;
    }

    if (PBZ_TextureDiffuseLength > 0) {
        albedo /= float(PBZ_TextureDiffuseLength);
    }

    float specular = 0.0f;

    for (uint i = 0; i < PBZ_TextureSpecularLength; ++i) {
        specular += texture(PBZ_TextureSpecular[i], fs_in.texCoord).r;
    }

    if (PBZ_TextureSpecularLength > 0) {
        specular /= float(PBZ_TextureSpecularLength);
    }

    vec3 height = vec3(0.0f);

    if (PBZ_TextureHeightLength > 0) {
        for (uint i = 0; i < PBZ_TextureHeightLength; ++i) {
            height += texture(PBZ_TextureHeight[i], fs_in.texCoord).rgb;
        }

        height /= float(PBZ_TextureHeightLength);
        height = height * 2.0 - 1.0;
        height = fs_in.TBN * height;
    } else {
        height = fs_in.normal;
    }

    PBZ_GBuffer0 = vec4(fs_in.fragPosition, 1.0f);
    PBZ_GBuffer1 = vec4(normalize(height), 1.0f);
    PBZ_GBuffer2 = vec4(albedo, specular);
}
