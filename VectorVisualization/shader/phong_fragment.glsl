varying vec3 varLightVec;
varying vec3 varViewVec;
varying vec3 varNormal;


void main(void)
{
    vec4 color = vec4(0);

    vec3 light = normalize(varLightVec);
    vec3 view  = normalize(varViewVec);
    vec3 normal = normalize(varNormal);

    vec3 halfway = normalize(view + light);

    float diffuse = max(dot(light, normal), 0.0);
    float specular = max(pow(dot(halfway, normal), gl_FrontMaterial.shininess), 0.0);

    // use gl_Color instead diffuse?
    color = gl_FrontMaterial.diffuse * (gl_FrontMaterial.ambient + diffuse)
        + gl_FrontMaterial.specular * specular;

    gl_FragColor = color;
}
