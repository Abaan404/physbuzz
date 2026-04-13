module point;

import "../../defines";
import "../../lighting/lights";

namespace Physbuzz {

[[vk::binding(0, 0)]]
StructuredBuffer<PointLight> points;

[[vk::binding(1, 0)]]
StructuredBuffer<InstanceData> objects;

struct VSInput {
    float3 position : POSITION;
};

struct VSOutput {
    float4 sv_position : SV_Position;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, uint instanceId: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation, uint viewIdx: SV_ViewID) {
    VSOutput output;
    uint objectId = instanceId + baseInstance;

    InstanceData object = objects[objectId];
    output.sv_position = mul(mul(float4(input.position, 1.0), object.model), points[0].projectionView[viewIdx]);

    return output;
}

}
