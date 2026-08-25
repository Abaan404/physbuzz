module collision_highlight;

import "builtin/defines";

namespace Physbuzz {

struct Highlighted {
    bool highlight;
};

[[vk::binding(0, 0)]]
ConstantBuffer<Camera> camera;

[[vk::binding(1, 0)]]
StructuredBuffer<Highlighted> highlightedInstances;

[[vk::binding(2, 0)]]
StructuredBuffer<InstanceData> objects;

struct VSInput {
    float3 position : POSITION;
};

struct CoarseVertex {
    float3 position;
    uint objectId;
};

struct VSOutput {
    float4 sv_position : SV_Position;
    CoarseVertex coarseVertex : COARSE_VERTEX;
};

[shader("vertex")]
VSOutput vertMain(VSInput input, uint instanceId: SV_InstanceID, uint baseInstance: SV_StartInstanceLocation) {
    VSOutput output;
    output.coarseVertex.objectId = instanceId + baseInstance;

    InstanceData object = objects[output.coarseVertex.objectId];
    output.sv_position = mul(mul(mul(float4(input.position, 1.0), object.model), camera.view), camera.projection);

    output.coarseVertex.position = mul(float4(input.position, 1.0), object.model).xyz;

    return output;
}

static const float MAX_REFLECTION_LOD = 10.0;

[shader("fragment")]
float4 fragMain(CoarseVertex coarseVertex: COARSE_VERTEX) : SV_TARGET {
    Highlighted highlighted = highlightedInstances[coarseVertex.objectId];

    if (!highlighted.highlight) {
        return float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    return float4(1.0f, 0.0f, 0.0f, 0.4f);
}

}

