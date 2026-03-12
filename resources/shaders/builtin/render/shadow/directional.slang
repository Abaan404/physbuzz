module directional;

import "../../lighting/lights";

namespace Physbuzz {

struct ObjectData {
    float4x4 model;
    float4x4 normal;
    uint materialIdx;
};

[[vk::binding(0, 0)]]
StructuredBuffer<DirectionalLight> directionals;

[[vk::binding(1, 0)]]
StructuredBuffer<ObjectData> objects;

struct VSInput {
    float3 position : POSITION;
};

struct VSOutput {
    float4 sv_position : SV_Position;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, uint instanceId: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation) {
    VSOutput output;
    uint objectId = instanceId + baseInstance;

    ObjectData object = objects[objectId];
    output.sv_position = mul(mul(float4(input.position, 1.0), object.model), directionals[0].projectionView);

    return output;
}

}
