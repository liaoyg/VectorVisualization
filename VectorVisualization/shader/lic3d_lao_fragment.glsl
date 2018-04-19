uniform sampler3D licVolumeSampler;
uniform int sampleNum;
uniform int pointNum;
uniform float maxRayLen;

const float PAI = 3.1415926;

float rand(vec2 co)
{
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 genPointonSphere(float theta, float u)
{
	float x = sqrt(1 - u*u) * cos(theta);
	float y = sqrt(1 - u*u) * sin(theta);
	float z = u;

	return vec3(x, y, z);
}

vec3 randomSphericalDistr(int i, int itrsPerAngle)
{
	int cur_angle = i / itrsPerAngle;
	int cur_itr = i % itrsPerAngle;
	float theta = rand(vec2(cur_angle, cur_itr)) * 2 * PAI;
	float u = 2 * rand(vec2(cur_itr, cur_angle)) - 1;
	return genPointonSphere(theta, u);
}

vec3 uniformSphericalDistr(int i, int angles, int itrsPerAngle)
{
	int idx_angle = i / itrsPerAngle;
	int idx_itrs = i % itrsPerAngle;
	float theta = 2 * PAI * (idx_angle + 0.5) / angles;
	float u = (idx_itrs + 0.5) / itrsPerAngle;
	return genPointonSphere(theta, u);
}

void main(void)
{
	vec4 geomPos = gl_TexCoord[0];
    vec3 pos = geomPos.xyz * scaleVol.xyz;
	// lookup scalar value
	vec4 illum = vec4(0.0, 0.0, 0.0, 1.0);
	
	float laoStepSize = maxRayLen * 1.0 / sampleNum;
	float aoKsum = 0.0;
	int angleNum = 12;
	int itrPerAngle = pointNum / angleNum;
#if defined(NOISE_LAO_COMPUTE)
	vec4 vectorData = texture3D(volumeSampler, pos);
	//if (vectorData.a > minScalarRange / maxVectorLength  && vectorData.a < maxScalarRange / maxVectorLength)
	{
#endif
		for (int i = 0; i < pointNum; i++)
		{
			//vec3 pointRay = uniformSphericalDistr(i, angleNum, itrPerAngle);
			vec3 pointRay = randomSphericalDistr(i, itrPerAngle);
			vec3 pointStart = pos;
			float alphaAcu = 0;
			for (int k = 0; k < sampleNum; k++)
			{
				vec3 pointPos = pointStart + pointRay*laoStepSize;
				// if noise data, intensity is in a, licvolume intensity is in r
#if defined(NOISE_LAO_COMPUTE)
				float intensity = texture3D(licVolumeSampler, pointPos).a;
#else
				float intensity = texture3D(licVolumeSampler, pointPos).r;
#endif
				float alpha = texture1D(transferAlphaOpacSampler, intensity*1.3).a;
				alphaAcu = alphaAcu*(1 - alpha) + alpha;
				if (alphaAcu >= 0.995)
					break;
			}
			aoKsum += alphaAcu;
			//aoKsum = alphaAcu / (i + 1) + aoKsum*(1.0 - 1.0 / (i + 1));
		}
		if (pointNum > 0)
			aoKsum /= pointNum;
		illum.rgb = vec3(aoKsum);
#if defined(NOISE_LAO_COMPUTE)
	}
#endif
	gl_FragColor = illum;
	//gl_FragColor = vec4(pos, 1);
}