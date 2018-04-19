#if defined(ILLUM_GRADIENT)
#   define USE_NOISE_GRADIENTS
#endif

void main(void)
{
    vec4 tfData;
    vec4 noise;
    vec4 illum;

    vec4 dest = texture2DRect(imageFBOSampler, gl_FragCoord.xy);
    vec4 src = vec4(0.0);

    if (dest.a < 0.95)
    {
        // compute the ray starting point
        vec4 geomPos = gl_TexCoord[0];
        vec3 pos = geomPos.xyz * scaleVol.xyz;

        // compute camera position by translating the origin to the volume center
        vec4 camera = gl_ModelViewMatrixInverse[3];

        // compute normalized ray direction
        vec4 geomDir = vec4(normalize(geomPos.xyz - camera.xyz), 0.0);
        vec3 dir = geomDir.xyz * scaleVol.xyz;

        //float scalarData;
        vec4 vectorData;

        // TODO: MC offset
#ifdef USE_MC_OFFSET
        pos += dir * stepSize * texture2DRect(mcOffsetSampler, gl_FragCoord.xy).r;
#endif

        // lookup scalar value
        vectorData = texture3D(volumeSampler, pos);
        /*
        // TODO: use lambda2 volume ...
        vectorData.a = texture3D(lambda2Sampler, pos).r;
        */

        // lookup in transfer function
        tfData = texture1D(transferRGBASampler, vectorData.a);

        // compute lic only if sample is visible
        if (tfData.a > 0.05)
        {
            // compute the LIC integral
			vec3 streamStart;
			vec3 streamEnd;
			vec2 streamDis;
            illum = computeLIC(pos, vectorData, streamDis, streamStart, streamEnd);
            //illum = computeLIC(pos, vectorData);

            // scale LIC intensity
            illum.a *= licKernel.b * gradient.r;

            // perform illumination
            // zoeckler/mallo/gradient/no illum
#if defined(ILLUM_GRADIENT)
            src = illumGradient(illum, tfData, pos, dir, vectorData.xyz);
#elif defined(ILLUM_MALLO)
            src = illumMallo(illum.a, tfData, pos, dir, vectorData.xyz);
#elif defined(ILLUM_ZOECKLER)
            src = illumZoeckler(illum.a, tfData, pos, dir, vectorData.xyz);
#else
            // -- standard LIC --
            src = illumLIC(illum.a, tfData);
#endif

            // perform blending
            src.rgb *= src.a;
            dest = clamp((1.0-dest.a)*src + dest, 0.0, 1.0);
        }
    }

    gl_FragColor = dest;
}
