module frustum;

namespace Physbuzz {

struct RenderData {
    float3[2] aabb;
    float4x4 transform;
    uint meshIdx;
}

struct DrawIndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int baseVertex;
    uint baseInstance;
};

struct PushConstants {
    float4 gPlanes[6];
    uint gObjectCount;
};

[[vk::binding(0, 0)]]
StructuredBuffer<RenderData> render;

[[vk::binding(1, 0)]]
RWStructuredBuffer<DrawIndexedIndirectCommand> draw;

[[vk::push_constant]]
PushConstants pc;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID) {
    DrawIndexedIndirectCommand command = draw[tid.x];

    draw[tid.x] = command;
}

}
