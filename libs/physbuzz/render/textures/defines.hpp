#pragma once

#include <assimp/scene.h>
#include <cstdint>

enum class TextureType : std::uint32_t {
    None = aiTextureType_NONE,
    Diffuse = aiTextureType_DIFFUSE,
    Specular = aiTextureType_SPECULAR,
    Ambient = aiTextureType_AMBIENT,
    Emissive = aiTextureType_EMISSIVE,
    Height = aiTextureType_HEIGHT,
    Normals = aiTextureType_NORMALS,
    Shininess = aiTextureType_SHININESS,
    Opacity = aiTextureType_OPACITY,
    Displacement = aiTextureType_DISPLACEMENT,
    Lightmap = aiTextureType_LIGHTMAP,
    Reflection = aiTextureType_REFLECTION,
    BaseColor = aiTextureType_BASE_COLOR,
    NormalCamera = aiTextureType_NORMAL_CAMERA,
    EmissionColor = aiTextureType_EMISSION_COLOR,
    Metalness = aiTextureType_METALNESS,
    DiffuseRoughness = aiTextureType_DIFFUSE_ROUGHNESS,
    AmbientOcclusion = aiTextureType_AMBIENT_OCCLUSION,
    Unknown = aiTextureType_UNKNOWN,
    Sheen = aiTextureType_SHEEN,
    Clearcoat = aiTextureType_CLEARCOAT,
    Transmission = aiTextureType_TRANSMISSION,
    MayaBase = aiTextureType_MAYA_BASE,
    MayaSpecular = aiTextureType_MAYA_SPECULAR,
    MayaSpecularColor = aiTextureType_MAYA_SPECULAR_COLOR,
    MayaSpecularRoughness = aiTextureType_MAYA_SPECULAR_ROUGHNESS,
    Max = AI_TEXTURE_TYPE_MAX,
};
