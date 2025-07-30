struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct BlinnPhong {
    Light light;

    float shadow;

    float spec;
    float diff;
    float attenuation;
};

vec3 calcBlinnPhong(BlinnPhong phong, vec3 mAlbedo, float mSpecular) {
    vec3 ambient = phong.light.ambient * mAlbedo;
    vec3 diffuse = phong.light.diffuse * phong.diff * mAlbedo;
    vec3 specular = phong.light.specular * phong.spec * mSpecular;

    return phong.attenuation * (ambient + (1.0 - phong.shadow) * (diffuse + specular));
}
