#version 460 core

#define MAX_POINT_LIGHTS 100
#define MAX_DIFFUSE_SAMPLERS 5
#define MAX_SPECULAR_SAMPLERS 5

#pbz_include "../common/lighting/point.glsl"
#pbz_include "../common/lighting/directional.glsl"
#pbz_include "../common/lighting/spot.glsl"

uniform uint u_TextureDiffuseLength;
uniform sampler2D u_TextureDiffuse[MAX_DIFFUSE_SAMPLERS];

uniform uint u_TextureSpecularLength;
uniform sampler2D u_TextureSpecular[MAX_SPECULAR_SAMPLERS];
uniform float u_Shininiess[MAX_SPECULAR_SAMPLERS];

uniform uint u_PointLightLength;
uniform PointLight u_PointLight[MAX_POINT_LIGHTS];

uniform Material u_Material;
uniform DirectionalLight u_DirectionalLight;
uniform SpotLight u_SpotLight;

uniform vec3 u_ViewPosition;

in vec3 normal;
in vec2 texCoord;
in vec3 fragPosition;

out vec4 fragColor;

void main() {
    vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < u_TextureDiffuseLength; i++) {
        result += texture(u_TextureDiffuse[i], texCoord);
    }

    // fragColor = texture(texture_diffuse[], TexCoords);

    // vec3 result;
    //
    // result += calcDirectionalLight(u_Material, u_DirectionalLight, fragPosition, normal, texCoord, u_ViewPosition);
    // result += calcSpotLight(u_Material, u_SpotLight, fragPosition, normal, texCoord, u_ViewPosition);
    //
    // for (int i = 0; i < u_PointLightLength; i++) {
    //     result += calcPointLight(u_Material, u_PointLight[i], fragPosition, normal, texCoord, u_ViewPosition);
    // }
    //
    fragColor = result;
}
