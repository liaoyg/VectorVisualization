
void main(void)
{
	vec4 geomPos = gl_TexCoord[0];
    vec3 pos = geomPos.xyz * scaleVol.xyz;
	// lookup scalar value
	vec4 scalarData = texture3D(scalarSampler, pos); 
    vec4 vectorData = texture3D(volumeSampler, pos);
	vec4 illum = vec4(0.0, 0.0, 0.0, 1.0);
	
	//if(pos.x < 0.5 && pos.y > 0.5)
	{
		illum.r = computeLIC(pos, vectorData).r;
		// scale LIC intensity
		illum.r *= licKernel.b * gradient.r;
		//illum = vec4(pos, 1);
	}

	gl_FragColor = illum;
	//gl_FragColor = vec4(pos, 1);
}