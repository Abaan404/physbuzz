implementing "../brdf";

import "../../defines.slang";

namespace Physbuzz {

public struct CookTorrence : IBRDF {
    public float3 albedo;
    public float3 irradiance;
    public float3 prefilteredColor;
    public float2 envBRDF;
    public float metallic;
    public float roughness;
    public float ambientOcclusion;

    public float3 evaluate(float3 incident, float3 exitant, float3 normal) {
        float3 halfway = normalize(incident + exitant);

        float NdotL = max(dot(normal, incident), 0.0f);
        float NdotV = max(dot(normal, exitant), 0.0f);

        float D = distribution(normal, halfway);
        float G = geometry(incident, exitant, normal);
        float3 F = fresnel(incident, halfway);

        float3 specular = (D * G * F) / max(4.0f * NdotL * NdotV, 1e-4f);

        // Energy conservation
        float3 ks = F;
        float3 kd = float3(1.0) - ks;
        kd *= 1.0 - metallic;

        float3 diffuse = kd * albedo / PI;

        return (diffuse + specular);
    }

    public float3 evaluateIndirect(float3 exitant, float3 normal) {
        float3 F = fresnelRoughness(exitant, normal);

        float3 ks = F;
        float3 kd = 1.0 - ks;
        kd *= 1.0 - metallic;

        float3 diffuse = kd * irradiance * albedo;

        float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

        return (diffuse + specular) * ambientOcclusion;
    }

    // Schlick 94 approximation
    private float3 fresnel(float3 incident, float3 microNormal) {
        float3 F0 = float3(0.04);

        F0 = lerp(F0, albedo, metallic);

        float cosTheta = max(dot(microNormal, incident), 0.0f);

        return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5);
    }

    // Schlick 94 approximation with injected roughness
    private float3 fresnelRoughness(float3 incident, float3 microNormal) {
        float3 F0 = float3(0.04);

        F0 = lerp(F0, albedo, metallic);

        float cosTheta = max(dot(microNormal, incident), 0.0f);

        // injecting a roughness term https://seblagarde.wordpress.com/2011/08/17/hello-world/
        return F0 + (max(float3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    // GGX/Trowbridge-Reitz Distribution
    private float distribution(float3 normal, float3 microNormal) {
        float NdotH = max(dot(microNormal, normal), 0.0f);

        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH2 = NdotH * NdotH;

        float nom = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / denom;
    }

    // Smith's method with Schlick-GGX approximation
    private float geometry(float3 incident, float3 exitant, float3 normal) {
        float NdotV = max(dot(normal, exitant), 0.0f);
        float NdotL = max(dot(normal, incident), 0.0f);

        float r = roughness + 1.0;
        float k = (r * r) / 8.0;

        float ggx1 = NdotV / (NdotV * (1.0 - k) + k);
        float ggx2 = NdotL / (NdotL * (1.0 - k) + k);

        return ggx1 * ggx2;
    }
};

}
