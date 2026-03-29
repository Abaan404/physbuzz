module frustum;

namespace Physbuzz {

struct SubmeshData {
    struct Bounding {
        float3 min;
        float3 max;
    }

    float4x4 model;
    float4x4 normal;
    uint materialIdx;
    Bounding bounding;
    bool isCulled;
}

struct ObjectData {
    float4x4 model;
    float4x4 normal;
    uint materialIdx;
}

struct DrawIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
}

struct StateData {
    uint instanceOffset;
}

struct PushConstants {
    float4 planes[6];
}

[[vk::binding(0, 0)]]
RWStructuredBuffer<SubmeshData> submeshes;

[[vk::binding(1, 0)]]
RWStructuredBuffer<ObjectData> objects;

[[vk::binding(2, 0)]]
RWStructuredBuffer<DrawIndexedIndirectCommand> indirect;

[[vk::binding(3, 0)]]
RWStructuredBuffer<StateData> state;

[[vk::push_constant]]
PushConstants pc;

float signedDistance(float4 plane, float3 point) {
    return dot(plane.xyz, point) + plane.w;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID) {
    DrawIndexedIndirectCommand draw = indirect[tid.x];

    uint firstInstance = draw.firstInstance;
    uint totalInstances = draw.instanceCount;
    uint culledCount = 0;

    for (uint j = 0; j < totalInstances; j++) {
        SubmeshData submesh = submeshes[firstInstance + j];

        // TODO use an OBB here to avoid computing a second rotated AABB
        float3[] corners = {
            float3(mul(float4(submesh.bounding.min.x, submesh.bounding.min.y, submesh.bounding.min.z, 1.0f), submesh.model).xyz),
            float3(mul(float4(submesh.bounding.min.x, submesh.bounding.max.y, submesh.bounding.min.z, 1.0f), submesh.model).xyz),
            float3(mul(float4(submesh.bounding.min.x, submesh.bounding.min.y, submesh.bounding.max.z, 1.0f), submesh.model).xyz),
            float3(mul(float4(submesh.bounding.min.x, submesh.bounding.max.y, submesh.bounding.max.z, 1.0f), submesh.model).xyz),
            float3(mul(float4(submesh.bounding.max.x, submesh.bounding.min.y, submesh.bounding.min.z, 1.0f), submesh.model).xyz),
            float3(mul(float4(submesh.bounding.max.x, submesh.bounding.max.y, submesh.bounding.min.z, 1.0f), submesh.model).xyz),
            float3(mul(float4(submesh.bounding.max.x, submesh.bounding.min.y, submesh.bounding.max.z, 1.0f), submesh.model).xyz),
            float3(mul(float4(submesh.bounding.max.x, submesh.bounding.max.y, submesh.bounding.max.z, 1.0f), submesh.model).xyz),
        };

        SubmeshData.Bounding aabb;
        aabb.min = float3(float.maxValue);
        aabb.max = float3(-float.maxValue);

        for (uint k = 0; k < 8; k++) {
            aabb.min = min(aabb.min, corners[k]);
            aabb.max = max(aabb.max, corners[k]);
        }

        float3 center = (aabb.min + aabb.max) / 2.0f;
        float radius = length(aabb.max - aabb.min) / 2.0f;

        submeshes[firstInstance + j].isCulled = false;

        for (uint k = 0; k < 6; k++) {
            if (signedDistance(pc.planes[k], center) < -radius) {
                // write back culling result, used after a range is found in the object buffer
                submeshes[firstInstance + j].isCulled = true;
                culledCount++;
                break;
            }
        }
    }

    InterlockedAdd(state[0].instanceOffset, draw.instanceCount, draw.firstInstance);
    draw.instanceCount -= culledCount;

    uint instanceId = 0;
    for (uint j = 0; j < totalInstances; j++) {
        SubmeshData submesh = submeshes[firstInstance + j];

        // only push objects that arent culled
        if (!submesh.isCulled) {
            objects[draw.firstInstance + instanceId] = {
                submesh.model,
                submesh.normal,
                submesh.materialIdx,
            };

            instanceId++;
        }
    }

    indirect[tid.x] = draw;
}

}
