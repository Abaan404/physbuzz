module defines;

namespace Physbuzz {

public struct Camera {
    public float3 position;
    public float4x4 view;
    public float4x4 projection;
};

public struct Material {
    public uint diffuseTextureId;
    public uint specularTextureId;
    public uint heightTextureId;
    public float specularity;

    public static const uint STRIDE = 16;
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

}
