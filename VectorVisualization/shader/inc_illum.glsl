vec4 illumGradient(in vec4 illum, in vec4 tfData,
                   in vec3 pos, in vec3 dir, in vec3 tangent)
{
    vec3 diffuse;
    vec3 specular;
    vec4 color;

    vec3 lightDir = normalize(gl_LightSource[0].position.xyz - pos * scaleVolInv.xyz);
    vec3 viewDir = normalize(-dir);

    vec3 normal = normalize(-illum.xyz);

    vec3 reflectDir = normalize(2.0*dot(lightDir, normal) * normal - lightDir);

    specular.r = clamp(dot(reflectDir, viewDir), 0.0, 1.0);
    specular.r = pow(specular.r, gl_LightSource[0].spotExponent);

    specular = specular.r * illum.a * gl_LightSource[0].specular.rgb;


    // color according to transfer function
    color.rgb = mix(vec3(0.0), tfData.rgb, illum.a);

    diffuse.r = clamp(dot(lightDir, normal), 0.0, 1.0);
    // consider illumination scaling
     diffuse = diffuse.r * gl_LightSource[0].diffuse.rgb * gradient.g;

    // apply illumination
    color.rgb = color.rgb * (diffuse + 0.3 + gl_LightSource[0].ambient.rgb) + specular;

    color.rgb *= gradient.g;


    // alpha affected by lic intensity
    color.a = texture1D(transferAlphaOpacSampler, illum.a*1.3).a * tfData.a;

    // opacity and color correction
    color.a = 1.0 - pow(1.0 - color.a, alphaCorrection);

    return color;
}



// illuminated stream lines according to Mallo et al.
vec4 illumMallo(in float illum, in vec4 tfData,
                in vec3 pos, in vec3 dir, in vec3 tangent)
{
    vec2 illumCoord;
    vec3 diffuse;
    vec3 specular;
    vec4 color;
    vec3 binormal;
    vec3 normal;
    vec3 halfway;
    vec2 tmp;

    vec4 lt;  // contains dot products between
              //   xy  ligthDir dot { normal, tangent }
              //   zw  halfway  dot { normal, tangent }

    vec3 lightDir = normalize(gl_LightSource[0].position.xyz - pos * scaleVolInv.xyz);
    vec3 viewDir = normalize(-dir);

    tangent = normalize(2.0*tangent - 1.0);

    binormal = normalize(cross(tangent, viewDir));
    normal = cross(binormal, tangent);

    halfway = normalize(viewDir + lightDir);

    lt.x = dot(lightDir, normal);
    lt.y = dot(lightDir, tangent);
    lt.z = dot(halfway, normal);
    lt.w = dot(halfway, tangent);

    // 1/sqrt( 1 / ({ lightVec, halfway } dot tangent)^2)
    tmp.x = 1.0 / sqrt(1.0 - lt.y*lt.y);
    tmp.y = 1.0 / sqrt(1.0 - lt.w*lt.w);
    lt.zw = lt.xz * tmp;

    lt = 0.5*lt + 0.5;

    // read diffuse factor
    diffuse = texture2D(malloDiffSampler, lt.zy).rgb;
    // read specular factor
    specular = texture2D(malloSpecSampler, lt.zw).rgb;

    // calculate specular attenuation
    // (1 - (halfway dot tangent)^2) ^ (exp/2)
    lt.w = pow(tmp.y, -gl_LightSource[0].spotExponent);
    specular = clamp(specular*lt.w, 0.0, 1.0);

    //specular *= 0.8;
    // consider illumination scaling
    diffuse *= gradient.g;

    // color according to transfer function
    color.rgb = mix(vec3(0.0), tfData.rgb, illum);

    // apply illumination
    color.rgb = color.rgb * diffuse + specular;

    // alpha affected by lic intensity
    color.a = texture1D(transferAlphaOpacSampler, illum*1.3).a * tfData.a;

    // opacity and color correction
    color.a = 1.0 - pow(1.0 - color.a, alphaCorrection);

    return color;
    //return vec4(normal*0.5+0.5, 1);
}



// illuminated stream lines according to Zoeckler et al.
vec4 illumZoeckler(in float illum, in vec4 tfData,
                   in vec3 pos, in vec3 dir, in vec3 tangent)
{
    vec2 illumCoord;
    vec2 shading;   // r diffuse,  g specular
    vec4 color;

    vec3 lightDir = normalize(gl_LightSource[0].position.xyz - pos * scaleVolInv.xyz);
    vec3 viewDir = normalize(dir);

    tangent = normalize(2.0*tangent - 1.0);

    illumCoord.x = dot(lightDir, tangent);
    illumCoord.y = dot(viewDir, tangent);
    illumCoord = 0.5*illumCoord + 0.5;

    shading = texture2D(zoecklerSampler, illumCoord).rg;
    // consider illumination scaling
    shading.r *= gradient.g;

    // usual black and white
    //color.rgb = vec3(illum);

    // color according to transfer function
    color.rgb = mix(vec3(0.0), tfData.rgb, illum);


    // apply illumination
    color.rgb = color.rgb * shading.r + 0.9*shading.g;

    // alpha affected by lic intensity
    color.a = texture1D(transferAlphaOpacSampler, illum*1.3).a * tfData.a;

    // opacity and color correction
    color.a = 1.0 - pow(1.0 - color.a, alphaCorrection);

    return color;
}



vec4 illumLIC(in float illum, in vec4 tfData)
{
    vec4 color;

    // result = lic intensity * color * illumination scaling
    color.rgb = illum * tfData.rgb * gradient.g;

    // alpha affected by lic intensity
    color.a = texture1D(transferAlphaOpacSampler, illum*1.3).a * tfData.a;

    // opacity and color correction
    color.a = 1.0 - pow(1.0 - color.a, alphaCorrection);

    return color;
}
