#pbz_include "defines.glsl"

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec4 calcDirectionalLight(DirectionalLight light, float shininess, vec3 fragPosition, vec3 normal, vec3 viewPosition, vec4 materialDiffuse, vec4 materialSpecular) {
    // diffuse
    float diff = max(dot(normal, -light.direction), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 halfwayDirection = normalize(light.direction + viewDirection);
    float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);

    // phong
    vec3 ambient = light.ambient * materialDiffuse.rgb;
    vec3 diffuse = light.diffuse * diff * materialDiffuse.rgb;
    vec3 specular = light.specular * spec * materialSpecular.rgb;

    return vec4(ambient + diffuse + specular, materialDiffuse.a);
}
