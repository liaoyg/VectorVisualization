#if defined(ILLUM_GRADIENT)
#   define USE_NOISE_GRADIENTS
#endif

layout(location = 0) out vec4 out_scalar;
layout(location = 1) out vec4 out_normal;

void main(void)
{
	vec4 geomPos = gl_TexCoord[0];
    vec3 pos = geomPos.xyz * scaleVol.xyz;
	// lookup scalar value
	//vec4 scalarData = texture3D(scalarSampler, pos); 
    vec4 vectorData = texture3D(volumeSampler, pos);
	//vec4 vectorDataNext = texture3D(volumeSamplerNext, pos);
	vec4 illum = vec4(0.0);
	//if(pos.x < 0.5 && pos.y > 0.5)
		vec3 streamStart;
		vec3 streamEnd;
		vec2 streamDis;
		vec4 res = computeLIC(pos, vectorData, streamDis, streamStart, streamEnd);
		illum = vec4(res.a * licKernel.b * gradient.r);
		illum.a = 1.0;

		// scale LIC intensity

		//illum.g = computeLIC(pos, vectorDataNext, streamDis, streamStart, streamEnd, volumeSamplerNext).r;
		// scale LIC intensity
		//illum.g *= licKernel.b * gradient.r;

		//if (vectorData.g == vectorDataNext.g)
		//if (illum.r == illum.g)
			//illum.b = 1.0;

		out_scalar = illum;
		out_normal = vec4(normalize(res.rgb), 0.0);
	//gl_FragColor = vec4(pos, 1);
}