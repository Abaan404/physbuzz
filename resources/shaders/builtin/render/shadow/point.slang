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

struct CoarseVertex {
    float3 position;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    CoarseVertex coarseVertex;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, uint instanceId: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation, uint viewIdx: SV_ViewID) {
    VSOutput output;
    uint objectId = instanceId + baseInstance;

    InstanceData object = objects[objectId];
    output.sv_position = mul(mul(float4(input.position, 1.0), object.model), points[0].projectionView[viewIdx]);

    output.coarseVertex.position = mul(float4(input.position, 1.0), object.model).xyz;

    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput output) : SV_Depth {
    float lightDistance = length(output.coarseVertex.position - points[0].position);
    return lightDistance / points[0].farPlane;
}

}
