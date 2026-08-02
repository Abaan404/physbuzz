module defines;

namespace Physbuzz {

public struct Camera {
    public float3 position;
    public float4x4 view;
    public float4x4 projection;
};

public struct Material {
    public uint albedoTextureId = -1;
    public uint normalTextureId = -1;
    public uint metallicTextureId = -1;
    public uint roughnessTextureId = -1;
    public uint emissionTextureId = -1;
    public uint ambientOcclusionTextureId = -1;

    public float metallicFactor = 0.0f;
    public float roughnessFactor = 0.0f;
    public float3 albedoFactor = float3(1.0);

    public static const uint STRIDE = 44;
};

public struct InstanceData {
    public struct Bounding {
        public float3 min;
        public float3 max;
    }

    public float4x4 model;
    public float4x4 normal;
    public uint materialIdx;
    public Bounding bounding;
};

public struct VisibilityData {
    public uint instanceId;
}

public static const float PI = 3.14159265359f;

}
