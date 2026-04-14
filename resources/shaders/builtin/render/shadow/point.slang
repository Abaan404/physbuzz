module point;

import "../../defines";
import "../../lighting/lights";

namespace Physbuzz {

struct PushConstants {
    uint objectBufferOffset;
}

[[vk::push_constant]]
PushConstants pc;

[[vk::binding(0, 0)]]
StructuredBuffer<PointLight> points;

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
VSOutput vertMain(VSInput input, uint instanceId: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation, uint viewId: SV_ViewID) {
    VSOutput output;
    uint objectId = culling[instanceId + baseInstance + (pc.objectBufferOffset * viewId)].instanceId;

    InstanceData object = objects[objectId];
    output.sv_position = mul(mul(float4(input.position, 1.0), object.model), points[0].projectionView[viewId]);

    return output;
}

}
