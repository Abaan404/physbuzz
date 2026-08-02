module spherical;

import "defines";

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

public float3 uvToDirection(float2 uv) {
    float phi = (uv.x - 0.5) * 2.0 * PI;
    float theta = (uv.y - 0.5) * PI;

    return normalize(float3(cos(theta) * cos(phi), sin(theta), cos(theta) * sin(phi)));
}

}
