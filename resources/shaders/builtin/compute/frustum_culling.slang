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

struct CameraData {
    float4 planes[6];
}

struct PushConstants {
    uint objectCount;
    uint drawCount;
}

[[vk::binding(0, 0)]]
StructuredBuffer<DrawIndexedIndirectCommand> draws;

[[vk::binding(1, 0)]]
StructuredBuffer<InstanceData> objects;

[[vk::binding(2, 0)]]
StructuredBuffer<CameraData> cameras;

[[vk::binding(3, 0)]]
RWStructuredBuffer<VisibilityData> visibleInstances;

[[vk::binding(4, 0)]]
RWStructuredBuffer<DrawIndexedIndirectCommand> outputDraws;

[[vk::push_constant]]
PushConstants pc;

float signedDistance(float4 plane, float3 point) {
    return dot(plane.xyz, point) + plane.w;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID) {
    uint drawId = tid.x;
    uint cameraId = tid.y;

    DrawIndexedIndirectCommand draw = draws[drawId];
    CameraData camera = cameras[cameraId];

    uint instanceId = 0;

    for (uint j = 0; j < draw.instanceCount; j++) {
        InstanceData object = objects[draw.firstInstance + j];

        float3 center = (object.bounding.min + object.bounding.max) / 2.0f;
        float radius = length(object.bounding.max - object.bounding.min) / 2.0f;

        bool isCulled =
            signedDistance(camera.planes[0], center) < -radius ||
            signedDistance(camera.planes[1], center) < -radius ||
            signedDistance(camera.planes[2], center) < -radius ||
            signedDistance(camera.planes[3], center) < -radius ||
            signedDistance(camera.planes[4], center) < -radius ||
            signedDistance(camera.planes[5], center) < -radius;

        if (!isCulled) {
            // a lookup table of indices to find instances when used with baseInstance + instanceId
            // each camera will have its own visible buffer
            visibleInstances[draw.firstInstance + instanceId + (pc.objectCount * cameraId)] = {
                draw.firstInstance + j
            };

            instanceId++;
        }
    }

    // this will also leave gaps in between submeshes, not ideal but this works for now
    draw.instanceCount = instanceId;

    // each camera will have its own culled draw buffer
    outputDraws[drawId + (pc.drawCount * cameraId)] = draw;
}

}
