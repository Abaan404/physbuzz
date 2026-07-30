module spherical;

namespace Physbuzz {

public float2 directionToUV(float3 direction) {
    static const float2 invAtan = float2(0.1591, 0.3183);

    float3 dir = normalize(direction);

    float2 uv = {
        atan2(dir.z, dir.x),
        asin(dir.y)
    };

    return uv * invAtan + 0.5;
}

}
