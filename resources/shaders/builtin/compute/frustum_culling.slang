module frustum_culling;

import "../defines";

namespace Physbuzz {

struct DrawIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
}

struct PushConstants {
    float4 planes[6];
}

[[vk::binding(0, 0)]]
RWStructuredBuffer<DrawIndexedIndirectCommand> indirect;

[[vk::binding(1, 0)]]
StructuredBuffer<InstanceData> objects;

[[vk::binding(2, 0)]]
RWStructuredBuffer<CullingData> culling;

[[vk::push_constant]]
PushConstants pc;

float signedDistance(float4 plane, float3 point) {
    return dot(plane.xyz, point) + plane.w;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID) {
    uint drawId = tid.x;
    DrawIndexedIndirectCommand draw = indirect[drawId];

    uint instanceId = 0;

    for (uint j = 0; j < draw.instanceCount; j++) {
        InstanceData object = objects[draw.firstInstance + j];

        // TODO use an OBB here to avoid computing a second rotated AABB
        float3[] corners = {
            float3(mul(float4(object.bounding.min.x, object.bounding.min.y, object.bounding.min.z, 1.0f), object.model).xyz),
            float3(mul(float4(object.bounding.min.x, object.bounding.max.y, object.bounding.min.z, 1.0f), object.model).xyz),
            float3(mul(float4(object.bounding.min.x, object.bounding.min.y, object.bounding.max.z, 1.0f), object.model).xyz),
            float3(mul(float4(object.bounding.min.x, object.bounding.max.y, object.bounding.max.z, 1.0f), object.model).xyz),
            float3(mul(float4(object.bounding.max.x, object.bounding.min.y, object.bounding.min.z, 1.0f), object.model).xyz),
            float3(mul(float4(object.bounding.max.x, object.bounding.max.y, object.bounding.min.z, 1.0f), object.model).xyz),
            float3(mul(float4(object.bounding.max.x, object.bounding.min.y, object.bounding.max.z, 1.0f), object.model).xyz),
            float3(mul(float4(object.bounding.max.x, object.bounding.max.y, object.bounding.max.z, 1.0f), object.model).xyz),
        };

        InstanceData.Bounding aabb;
        aabb.min = float3(float.maxValue);
        aabb.max = float3(-float.maxValue);

        for (uint k = 0; k < 8; k++) {
            aabb.min = min(aabb.min, corners[k]);
            aabb.max = max(aabb.max, corners[k]);
        }

        float3 center = (aabb.min + aabb.max) / 2.0f;
        float radius = length(aabb.max - aabb.min) / 2.0f;

        bool isCulled =
            signedDistance(pc.planes[0], center) < -radius ||
            signedDistance(pc.planes[1], center) < -radius ||
            signedDistance(pc.planes[2], center) < -radius ||
            signedDistance(pc.planes[3], center) < -radius ||
            signedDistance(pc.planes[4], center) < -radius ||
            signedDistance(pc.planes[5], center) < -radius;

        if (!isCulled) {
            // a lookup table of indices to find instances when used with baseInstance + instanceId
            culling[draw.firstInstance + instanceId] = {
                draw.firstInstance + j
            };

            instanceId++;
        }
    }

    // this will also leave gaps in between submeshes, not ideal but this works for now
    draw.instanceCount = instanceId;

    indirect[drawId] = draw;
}

}
