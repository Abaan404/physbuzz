#pbz_include "blinnphong.glsl"

#define MAX_SPOT_LIGHTS 100

struct SpotLight {
    Light light;

    vec3 position;
    vec3 direction;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};

BlinnPhong calcSpotLight(SpotLight spot, vec3 fragPosition, vec3 viewPosition, vec3 normal, float shininess) {
    BlinnPhong phong;
    phong.light = spot.light;

    // diffuse
    vec3 lightDirection = normalize(spot.position - fragPosition);
    phong.diff = max(dot(normal, lightDirection), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDirection, normal);
    phong.spec = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);

    float theta = dot(lightDirection, normalize(-spot.direction));
    float epsilon = spot.cutOff - spot.outerCutOff;
    float intensity = clamp((theta - spot.outerCutOff) / epsilon, 0.0, 1.0);

    // inverse square law
    float distance = length(spot.position - fragPosition);
    phong.attenuation = 1.0 / (spot.constant + spot.linear * distance + spot.quadratic * (distance * distance));

    phong.light.diffuse *= intensity;
    phong.light.specular *= intensity;

    phong.shadow = 0.0f;

    return phong;
}
