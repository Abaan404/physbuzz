#version 460 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;

    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material u_Material;
uniform Light u_Light;

uniform vec3 u_ViewPosition;

in vec3 normal;
in vec2 texCoord;
in vec3 fragPosition;

out vec4 fragColor;

void main() {
    // diffuse
    vec3 lightDir = normalize(u_Light.position - fragPosition);
    float diff = max(dot(normal, lightDir), 0.0);

    // specular
    vec3 viewDir = normalize(u_ViewPosition - fragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);

    // phong
    vec3 ambient = u_Light.ambient * vec3(texture(u_Material.diffuse, texCoord));
    vec3 diffuse = u_Light.diffuse * diff * vec3(texture(u_Material.diffuse, texCoord));
    vec3 specular = u_Light.specular * spec * vec3(texture(u_Material.specular, texCoord));

    vec3 result = ambient + diffuse + specular;

    fragColor = vec4(result, 1.0);
}
