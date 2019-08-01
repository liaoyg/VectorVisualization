
uniform sampler3D licVolumeSampler;
uniform sampler3D licVolumeNormal;
uniform sampler3D laoVolumeSampler;
uniform sampler3D licVolumeNormalSampler;

uniform int interpSize;
uniform float interpStep;

void main(void)
{
    bool outside = false;
    vec4 data;
	vec4 tfData;
    vec4 noise;
	vec4 bgColor = vec4(1.0, 1.0, 1.0, 0.0);

    // compute the ray starting point
    vec4 geomPos = gl_TexCoord[0];
    vec3 pos = geomPos.xyz * scaleVol.xyz;

    // compute camera position by translating the origin to the volume center
    vec4 camera = gl_ModelViewMatrixInverse[3];

    // compute normalized ray direction
    vec4 geomDir = vec4(normalize(geomPos.xyz - camera.xyz), 0.0);
    vec3 dir = geomDir.xyz * scaleVol.xyz;

    float scalarData;
    vec4 volumeData;
	vec4 vectorData;
	vec4 normalData;
	float ao = 1.0;

    vec4 dest = vec4(0.0);
    vec4 src;

    for (int j=0; (!outside && (j<numIterations)); ++j)
    {
	//if(pos.x >= 0.5)
        for (int i=0; i<numIterations; ++i)
        {
            // lookup scalar value
            vectorData = texture3D(volumeSampler, pos);
            volumeData = texture3D(licVolumeSampler, pos);
			//normalData = texture3D(licVolumeNormalSampler, pos);
			normalData = vec4(sampleGrad2(licVolumeSampler, pos), 1.0);
			vec4 intensity = vec4(normalData.rgb, volumeData.r);
			//float intensity = normalData.r ;
			//volumeData.r *= licKernel.b * gradient.r;	
			//float scalarData = texture3D(scalarSampler, pos).r;

            // lookup in transfer function
			//vec3 center = vec3(0.76, 0.5, 0.5);
			//if(distance(pos, center) < 0.232)
			//if(pos.x >=0.5)
			if (vectorData.a > 0.1)
			{
#if defined(AMBIENT_OCCULUSION)
				ao = texture3D(laoVolumeSampler, pos).r;
#endif
				//tfData = vec4(0.0, 1.0, 0.0, 1.0);
				//tfData = vec4(vectorData.rgb, 1.0);

				float scalarROI = texture3D(vectorSampler, pos).a;
				//float scalarROI = texture3D(scalarSampler, pos).r;
				//tfData = texture1D(transferRGBASampler, scalarROI);
				tfData = texture1D(transferRGBASampler, vectorData.a);
#if defined(ILLUM_GRADIENT)
				//intensity = vec4(normalize(intensity.rgb), intensity.a);
				src = illumGradient(intensity, tfData, pos, dir, vectorData.xyz);
				//src = vec4(normalize(intensity.rgb), 1.0);
#elif defined(ILLUM_MALLO)
				src = illumMallo(intensity.a, tfData, pos, dir, vectorData.xyz);
#elif defined(ILLUM_ZOECKLER)
				src = illumZoeckler(intensity.a, tfData, pos, dir, vectorData.xyz);
#elif defined(AMBIENT_OCCULUSION)
				//src = illumLIC(intensity.a, tfData, ao);
				bool showLAO = false;
				if (showLAO)
				{
					tfData = vec4(0.8, 0.8, 0.8, 1.0);
					//src = vec4(tfData * (ao));
					if (ao > 0.0)
						src = vec4(tfData.rgb * (1.0 - ao), ao);
					else
						src = vec4(tfData.rgb * ao, 0.0);
					src.a = 1.0 - pow(1.0 - src.a, alphaCorrection);
					//src = illumLIC(ao, tfData);

				}
				else
				{
					ao = clamp(ao, 0.0, 1.0);
					src = illumGradient(intensity, tfData*ao, pos, dir, vectorData.xyz);
					//src.rgb = vec3(0.25)*ao;
				}
#else
				// -- standard LIC --
				src = illumLIC(intensity.a, tfData);
#endif
				//src = illumLIC(intensity, tfData);
				//src = volumeData;

				// perform blending
				src.rgb *= src.a;
				dest = clamp((1.0 - dest.a)*src + dest, 0.0, 1.0);
			}
            // move one step forward
            pos += dir * stepSize;
			// early ray termination with high opacity
            // terminate loop if outside volume
            outside = any(bvec4(clamp(pos.xyz, vec3(0.0), texMax.xyz) - pos.xyz, dest.a > 0.95));
            if (outside)
                break;
        }
    }
	dest = clamp((1.0-dest.a)*bgColor + dest, 0.0, 1.0);
    gl_FragColor = dest;
    //gl_FragColor = vec4(texture1D(transferRGBASampler, pos.x).rgb, 0.5);
}
