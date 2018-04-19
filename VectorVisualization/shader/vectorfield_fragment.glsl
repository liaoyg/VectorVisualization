// program parameters

// max texture coords for bounding box test
uniform vec4 texMax;

// scale factors of the volume
uniform vec4 scaleVol;

uniform float stepSize;
uniform vec3 gradient; // scale, illum scale, frequency scale

uniform int numIterations;

uniform float alphaCorrection;

uniform vec4 licParams;


// textures (have to be uniform)

uniform sampler3D volumeSampler;
uniform sampler3D noiseSampler;

uniform sampler2D mcOffsetSampler;

uniform sampler1D transferRGBASampler;
uniform sampler1D transferAlphaOpacSampler;

uniform sampler1D licKernelSampler;


uniform sampler2D malloDiffSampler;
uniform sampler2D malloSpecSampler;
uniform sampler2D zoecklerSampler;




void main(void)
{
    bool outside = false;
    vec4 data;
    vec4 noise;

    // compute the ray starting point
    vec4 geomPos = gl_TexCoord[0];
    vec3 pos = geomPos.xyz * scaleVol.xyz;

    // compute camera position by translating the origin to the volume center
    vec4 camera = gl_ModelViewMatrixInverse[3];

    // compute normalized ray direction
    vec4 geomDir = vec4(normalize(geomPos.xyz - camera.xyz), 0.0);
    vec3 dir = geomDir.xyz * scaleVol.xyz;

    float scalarData;
    vec4 vectorData;

    vec4 dest = vec4(0.0);
    vec4 src;

    //dest = texture3D(volumeSampler, pos);

    // move one step forward
    //pos += dir * stepSize;

    for (int j=0; (!outside && (j<numIterations)); ++j)
    {
        for (int i=0; i<numIterations; ++i)
        {
            // lookup scalar value
            vectorData = texture3D(volumeSampler, pos);
            noise = texture3D(noiseSampler, pos);
            scalarData = vectorData.r;

            // lookup in transfer function
            data = texture1D(transferRGBASampler, scalarData);

            //src = vec4(vectorData.xyz, data.a);
            src = vec4(data.xyz, vectorData.r);

            // perform blending
            src.rgb *= src.a;
            dest = clamp((1.0-dest.a)*src + dest, 0.0, 1.0);

            // early ray termination with high opacity
            // ...

            // move one step forward
            pos += dir * stepSize;

            // terminate loop if outside volume
            outside = any(bvec4(clamp(pos.xyz, vec3(0.0), texMax.xyz) - pos.xyz, dest.a > 0.95));
            if (outside)
                break;
        }
    }

    gl_FragColor = dest;
    //gl_FragColor = vec4(texture1D(transferRGBASampler, pos.x).rgb, 0.5);
}
