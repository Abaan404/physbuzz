#pbz_include "defines.glsl"

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

vec4 calcDirectionalLight(Material material, DirectionalLight light, vec3 fragPosition, vec3 normal, vec3 viewPosition, vec4 materialDiffuse, vec4 materialSpecular) {
    // diffuse
    float diff = max(dot(normal, -light.direction), 0.0);

    // specular
    vec3 viewDirection = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-light.direction, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), material.shininess);

    // phong
    vec3 ambient = light.ambient * materialDiffuse.rgb;
    vec3 diffuse = light.diffuse * diff * materialDiffuse.rgb;
    vec3 specular = light.specular * spec * materialSpecular.rgb;

    return vec4(ambient + diffuse + specular, materialDiffuse.a);
}
