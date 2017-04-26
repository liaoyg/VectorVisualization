
uniform sampler3D licVolumeSampler;
uniform sampler3D licVolumeSamplerOld;

uniform int interpSize;
uniform float interpStep;

void main(void)
{
    bool outside = false;
    vec4 data;
	vec4 tfData;
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
    vec4 volumeData;
	vec4 volumeDataOld;
	vec4 vectorData;

    vec4 dest = vec4(0.0);
    vec4 src;

    //dest = texture3D(licVolumeSampler, pos);

    // move one step forward
    //pos += dir * stepSize;

    for (int j=0; (!outside && (j<numIterations)); ++j)
    {
	//if(pos.x >= 0.5)
        for (int i=0; i<numIterations; ++i)
        {
            // lookup scalar value
            vectorData = texture3D(volumeSampler, pos);
            volumeData = texture3D(licVolumeSampler, pos);

			volumeDataOld = texture3D(licVolumeSamplerOld, pos);

			float intensity = volumeData.r;
			intensity = volumeDataOld.r*(1-interpStep) + volumeData.r * interpStep;
			//volumeData = volumeDataMix;
			//if(volumeData == volumeDataOld)
				//intensity = interpStep;

            //noise = texture3D(noiseSampler, pos);

            // lookup in transfer function
            tfData = texture1D(transferRGBASampler, vectorData.z);

            //src = vec4(tfData.xyz, volumeData.a);
            //src = vec4(noise.xyz, data.a);
			src = illumLIC(intensity, tfData);
			//src = volumeData;

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
