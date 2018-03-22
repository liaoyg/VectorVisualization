#define USE_NOISE_GRADIENTS

void main(void)
{
	vec4 geomPos = gl_TexCoord[0];
    vec3 pos = geomPos.xyz * scaleVol.xyz;
	// lookup vector value
    vec4 vectorData = texture3D(volumeSampler, pos);
	vec4 illum = vec4(0.0);
	vec3 streamStart;
	vec3 streamEnd;
	vec2 streamDis;
	vec4 res = computeLIC(pos, vectorData, streamDis, streamStart, streamEnd).rgba;
	illum.rgb = res.rgb;
	illum.a = 1.0;
	gl_FragColor = illum;
}