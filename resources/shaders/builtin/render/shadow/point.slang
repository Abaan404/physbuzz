module point;

import "../../defines";
import "../../lighting/lights";

namespace Physbuzz {

struct PushConstants {
    uint objectCount;
    uint cameraId;
    uint faceId;
}

[[vk::push_constant]]
PushConstants pc;

[[vk::binding(0, 0)]]
StructuredBuffer<PointLight> points;

[[vk::binding(1, 0)]]
StructuredBuffer<InstanceData> objects;

[[vk::binding(2, 0)]]
StructuredBuffer<VisibilityData> visibleInstances;

struct VSInput {
    float3 position : POSITION;
}

struct VSOutput {
    float4 sv_position : SV_Position;
}

[shader("vertex")]
VSOutput vertMain(VSInput input, uint instanceId: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation) {
    VSOutput output;
    uint objectId = visibleInstances[instanceId + baseInstance + (pc.objectCount * pc.cameraId)].instanceId;

    InstanceData object = objects[objectId];
    output.sv_position = mul(mul(float4(input.position, 1.0), object.model), points[0].projectionView[pc.faceId]);

    return output;
}

}
