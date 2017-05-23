void main(void)
{
	vec4 geomPos = gl_TexCoord[0];
    vec3 pos = geomPos.xyz * scaleVol.xyz;
	// lookup scalar value
	vec4 scalarData = texture3D(scalarSampler, pos); 
    vec4 vectorData = texture3D(volumeSampler, pos);
	vec4 vectorDataNext = texture3D(volumeSamplerNext, pos);
	vec4 illum = vec4(0.0, 0.0, 0.0, 1.0);
	
	//if(pos.x < 0.5 && pos.y > 0.5)
	{
		vec3 streamStart;
		vec3 streamEnd;
		vec2 streamDis;
		illum.r = computeLIC(pos, vectorData, streamDis, streamStart, streamEnd, volumeSampler).r;
		// scale LIC intensity
		illum.r *= licKernel.b * gradient.r;

		illum.g = computeLIC(pos, vectorDataNext, streamDis, streamStart, streamEnd, volumeSamplerNext).r;
		// scale LIC intensity
		illum.g *= licKernel.b * gradient.r;

		//if (vectorData.g == vectorDataNext.g)
		//if (illum.r == illum.g)
			//illum.b = 1.0;
	}

	gl_FragColor = illum;
	//gl_FragColor = vec4(pos, 1);
}