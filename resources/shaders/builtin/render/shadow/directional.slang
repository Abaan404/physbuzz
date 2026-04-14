module directional;

import "../../defines";
import "../../lighting/lights";

namespace Physbuzz {

[[vk::binding(0, 0)]]
StructuredBuffer<DirectionalLight> directionals;

[[vk::binding(1, 0)]]
StructuredBuffer<InstanceData> objects;

[[vk::binding(2, 0)]]
StructuredBuffer<CullingData> culling;

struct VSInput {
    float3 position : POSITION;
}

struct VSOutput {
    float4 sv_position : SV_Position;
}

[shader("vertex")]
VSOutput vertMain(VSInput input, uint instanceId: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation) {
    VSOutput output;
    uint objectId = culling[instanceId + baseInstance].instanceId;

    InstanceData object = objects[objectId];
    output.sv_position = mul(mul(float4(input.position, 1.0), object.model), directionals[0].projectionView);

    return output;
}

}
