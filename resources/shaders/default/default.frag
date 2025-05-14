#version 460 core

#pbz_include "../common/lighting/point.glsl"
#pbz_include "../common/lighting/directional.glsl"
#pbz_include "../common/lighting/spot.glsl"

#define MAX_DIFFUSE_SAMPLERS 5
#define MAX_SPECULAR_SAMPLERS 5
#define MAX_POINT_LIGHTS 100

layout(std140, binding = 1) uniform Camera {
    vec3 position;
    mat4 view;
    mat4 projection;
} camera;

uniform Material u_Material;
uniform sampler2D u_MaterialDiffuse[MAX_DIFFUSE_SAMPLERS];
uniform sampler2D u_MaterialSpecular[MAX_SPECULAR_SAMPLERS];

uniform uint u_PointLightLength;
uniform PointLight u_PointLight[MAX_POINT_LIGHTS];

uniform DirectionalLight u_DirectionalLight;
uniform SpotLight u_SpotLight;

uniform samplerCube u_Skybox;

in VS_OUT {
    vec3 normal;
    vec2 texCoord;
    vec3 fragPosition;
} fs_in;

out vec4 fragColor;

void main() {
    vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    // average the diffuse textures
    vec4 diffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < u_Material.diffuseLength; i++) {
        diffuse += texture(u_MaterialDiffuse[0], fs_in.texCoord);
    }
    diffuse /= float(u_Material.diffuseLength);

    // average the specular textures
    vec4 specular = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < u_Material.specularLength; i++) {
        specular += texture(u_MaterialSpecular[0], fs_in.texCoord);
    }
    specular /= float(u_Material.specularLength);

    result += calcDirectionalLight(u_Material, u_DirectionalLight, fs_in.fragPosition, fs_in.normal, camera.position, diffuse, specular);
    result += calcSpotLight(u_Material, u_SpotLight, fs_in.fragPosition, fs_in.normal, camera.position, diffuse, specular);

    vec3 I = normalize(fs_in.fragPosition - camera.position);
    vec3 R = refract(I, -normalize(fs_in.normal), 1.00 / 1.52);
    result += vec4(texture(u_Skybox, R).rgb, 1.0);

    for (int i = 0; i < u_PointLightLength; i++) {
        result += calcPointLight(u_Material, u_PointLight[i], fs_in.fragPosition, fs_in.normal, camera.position, diffuse, specular);
    }

    fragColor = result;
}
