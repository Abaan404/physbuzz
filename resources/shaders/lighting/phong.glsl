#pbz_include "defines.glsl"

vec3 calculatePhong(Material material, Light light, vec3 fragPosition, vec3 normal, vec2 texCoord, vec3 viewPosition) {
    // diffuse
    vec3 lightDir = normalize(light.position - fragPosition);
    float diff = max(dot(normal, lightDir), 0.0);

    // specular
    vec3 viewDir = normalize(viewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // phong
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, texCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, texCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, texCoord));

    return ambient + diffuse + specular;
}
