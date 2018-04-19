#extension GL_ARB_shader_image_size : enable

uniform layout(binding=3, rgba16f)  image3D scatterVolume;

vec3 nextStreamline(in vec3 pos)
{
	vec4 k1 = texture3D(volumeSampler, pos);
	vec4 k2 = texture3D(volumeSampler, pos + 0.5 * stepSize * k1.xyz);
	vec4 k3 = texture3D(volumeSampler, pos + 0.5 * stepSize * k2.xyz);
	vec4 k4 = texture3D(volumeSampler, pos + stepSize * k3.xyz);

	return pos + stepSize / 6 * (k1.xyz + 2 * k2.xyz + 2 * k3.xyz + k4.xyz);
}

void main(void)
{
/*
	vec4 geomPos = gl_TexCoord[0];
    vec3 pos = geomPos.xyz * scaleVol.xyz;
	// lookup scalar value
	vec4 scalarData = texture3D(scalarSampler, pos); 
    vec4 vectorData = texture3D(volumeSampler, pos);
	vec4 vectorDataNext = texture3D(volumeSamplerNext, pos);
	vec4 illum = vec4(0.0, 0.0, 0.0, 1.0);

	// scattering current point value to streamline
	// forward
	vec3 originPos = pos;
	int numIntegrate = 10;
	ivec3 size = imageSize(scatterVolume);
	//int x = gl_VertexID % size.x;
	//int y = ( gl_VertexID % (size.x * size.y)) / size.x;
	//int z = gl_VertexID / (size.x * size.y);
	//ivec3 i = ivec3(x, y , z);
	//originPos = vec3(x/256.0, y/256.0, z/256.0);
	for(int itr = 0; itr < numIntegrate; itr++)
	{
		originPos = nextStreamline(originPos);
		vec4 weightValue = 1.5 * texture3D(noiseSampler, pos*gradient.z);
		imageStore(scatterVolume,  ivec3(pos*size), weightValue ); 
	}
*/
	
	//gl_FragColor = illum;
}