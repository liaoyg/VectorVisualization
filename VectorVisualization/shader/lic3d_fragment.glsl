#if defined(ILLUM_GRADIENT)
#   define USE_NOISE_GRADIENTS
#endif

void main(void)
{
    bool outside = false;
    vec4 tfData;
    vec4 noise;
    vec4 illum;
	vec4 bgColor = vec4(1.0, 1.0, 1.0, 0.0);

	float ao = 1.0;


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

    vec4 dest = vec4(0.0);
    vec4 src = vec4(0.0);
	float dis = 0.0;
	vec3 oldPos = pos;

	//variable used for streamDis
	vec3 prestreamStart = vec3(0.0);
	vec3 prestreamEnd = vec3(0.0);
	vec2 prestreamDis = vec2(0.0);


    // TODO: MC offset
#ifdef USE_MC_OFFSET
    pos += dir * stepSize * texture2DRect(mcOffsetSampler, gl_FragCoord.xy).r;
#endif

    // move one step forward
    //pos += dir * stepSize;
#if 1
    for (int j=0; (!outside && (j<numIterations)); ++j)
    {
        for (int i=0; i<numIterations; ++i)
        {

            // lookup scalar value
            vectorData = texture3D(volumeSampler, pos);
            /*
            // TODO: use lambda2 volume ...
            vectorData.a = texture3D(lambda2Sampler, pos).r;
            */

            // lookup in transfer function
			// use secondary scalar data to map color value
			vec4 scalarData = texture3D(scalarSampler, pos); 
            tfData = vec4(vectorData.rgb, 1.0);

#if defined(STREAMLINE_DISTANCE)
			if(vectorData.a > 0.5 && vectorData.a < 0.75)
#else
			if (scalarData.g > -0.01)
#endif
            {
                // compute the LIC integral
				vec3 streamStart;
				vec3 streamEnd;
				vec2 streamDis;
                //illum = computeLIC(pos, vectorData, streamDis, streamStart, streamEnd);

				vec3 center = vec3(0.0, 0.0, 0.0);
				vec3 centerB = vec3(0.7, 0.7, 0.7);
				//vec3 center = pos * 8;
				//center = vec3(int(center.x)/8.0, int(center.y)/8.0, int(center.z)/8.0);

#if defined(STREAMLINE_DISTANCE)
				float strDis = computeStreamlineDis(pos, center);
				dis += strDis;
				illum.a = cos(strDis*256) - 0.5;
				if(illum.a > 0.0)
					illum.a = 1.0;
#else
				ao = 1.0;
				//illum = computeLIC(pos, vectorData, streamDis, streamStart, streamEnd);
                illum = computeLICwithAO(pos, vectorData, streamDis, streamStart, streamEnd, ao);
#endif
				//tfData = vec4(vectorData.rgb, 1.0);
				//ao *= licKernel.b * 15;
				ao = texture3D(noiseLAOSampler, pos).r;
				//ao = texture3D(noiseSampler, pos).a;
				
				oldPos = pos;

                // scale LIC intensity
                illum.a *= licKernel.b * gradient.r;

                // perform illumination
                // zoeckler/mallo/gradient/no illum
#if defined(ILLUM_GRADIENT)
                src = illumGradient(illum, tfData, pos, dir, vectorData.xyz);
				//src = vec4(normalize(illum.rgb), 1.0);
#elif defined(ILLUM_MALLO)
                src = illumMallo(illum.a, tfData, pos, dir, vectorData.xyz);
#elif defined(ILLUM_ZOECKLER) 
                src = illumZoeckler(illum.a, tfData, pos, dir, vectorData.xyz);
#elif defined(AMBIENT_OCCULUSION)

				//src = illumLIC(intensity.a, tfData, ao);
				tfData = vec4(1.0, 1.0, 1.0, 1.0);
				//src = vec4(tfData * (ao));
				if (ao > 0.0)
					src = vec4(tfData.rgb * (1-ao), ao);
				else
					src = vec4(tfData.rgb * (1 - ao), 0.0);
				//ao = clamp(1-ao, 0.0, 1.0);
				//src = illumGradient(illum, tfData, pos, dir, vectorData.xyz, ao);
#else
                // -- standard LIC --
                src = illumLIC(illum.a, tfData);
#endif
				//src = vec4(normalize(vectorData.rgb), illum.a);
                // perform blending
                src.rgb *= src.a;
                dest = clamp((1.0-dest.a)*src + dest, 0.0, 1.0);
            }

            // move one step forward
            pos += dir * stepSize;

            // terminate loop if outside volume and early ray termination
            outside = any(bvec4(clamp(pos.xyz, vec3(0.0), texMax.xyz) - pos.xyz, src.a > 0.95));
            if (outside)
                break;
        }
    }
#endif

	dest = clamp((1.0-dest.a)*bgColor + dest, 0.0, 1.0); // make up to white background
    gl_FragColor = dest;
}
