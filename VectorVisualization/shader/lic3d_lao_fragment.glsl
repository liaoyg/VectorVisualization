uniform sampler3D licVolumeSampler;
uniform sampler3D licVolumeNormalSampler;
uniform int sampleNum;
uniform int pointNum;
uniform float maxRayLen;

const float PAI = 3.1415926;

float rand(vec2 co)
{
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec3 sampleGrad(sampler3D tsampler, vec3 coord)
{
	const int offset = 1;
	float dx = textureOffset(tsampler, coord, ivec3(offset, 0, 0)).r - textureOffset(tsampler, coord, ivec3(-offset, 0, 0)).r;
	float dy = textureOffset(tsampler, coord, ivec3(0, offset, 0)).r - textureOffset(tsampler, coord, ivec3(0, -offset, 0)).r;
	float dz = textureOffset(tsampler, coord, ivec3(0, 0, offset)).r - textureOffset(tsampler, coord, ivec3(0, 0, -offset)).r;
	return normalize(vec3(dx, dy, dz));
}

vec3 sampleGrad2(sampler3D tsampler, vec3 coord)
{
	int[3][3][3][3] weights =
	{
		{ { { -1, -3, -1 },
		    { -3, -6, -3 },
		    { -1, -3, -1 } },
		  { { 0,  0,  0 },
		    { 0,  0,  0 },
		    { 0,  0,  0 } },
		  { { 1,  3,  1 },
		    { 3,  6,  3 },
		    { 1,  3,  1 } } },
		{ { { -1, -3, -1 },
			{ 0,  0,  0 },
			{ 1,  3,  1 } },
		  { { -3, -6, -3 },
			{ 0,  0,  0 },
			{ 3,  6,  3 } },
		  { { -1, -3, -1 },
			{ 0,  0,  0 },
			{ 1,  3,  1 } } },
		{ { { -1,  0,  1 },
			{ -3,  0,  3 },
			{ -1,  0,  1 } },
		  { { -3,  0,  3 },
		    { -6,  0,  6 },
			{ -3,  0,  3 } },
		  { { -1,  0,  1 },
			{ -3,  0,  3 },
			{ -1,  0,  1 } } }
	};
	float gp[3];
	for (int dir = 0; dir < 3; ++dir)
	{
		gp[dir] = 0.0f;
		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				for (int k = -1; k < 2; k++)
				{
					gp[dir] += weights[dir][i + 1][j + 1][k + 1] *
						textureOffset(tsampler, coord, ivec3(i, j, k)).r;
				}
			}
		}
		gp[dir] /= 2.0f;
	}

	return normalize(vec3(gp[0], gp[1], gp[2]));
}

mat3 rotationMatrix(vec3 n)
{
	float sinA = n.y / sqrt(n.x*n.x + n.y*n.y);
	float cosA = n.x / sqrt(n.x*n.x + n.y*n.y);
	float sinB = sqrt(n.x*n.x + n.y*n.y) / sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
	float cosB = n.z / sqrt(n.x*n.x + n.y*n.y + n.z*n.z);

	mat3 rot;
	rot[0].xyz = vec3(cosA*cosB, -sinA, cosA*sinB);
	rot[1].xyz = vec3(sinA*cosB, cosA, sinA*sinB);
	rot[2].xyz = vec3(-sinB, 0, cosB);
	return rot;
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
	float u = 2.0 * rand(vec2(cur_itr, cur_angle)) - 1.0;
	return genPointonSphere(theta, u);
}

vec3 uniformSphericalDistr(int i, int angles, int itrsPerAngle)
{
	int idx_angle = i / itrsPerAngle;
	int idx_itrs = i % itrsPerAngle;
	float theta = 2 * PAI * (idx_angle + 0.5) / angles;
	float u = 2.0 * (idx_itrs + 0.5) / itrsPerAngle - 1.0;
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
	int angleNum = 24;
	int itrPerAngle = pointNum / angleNum;
	vec4 vectorData = texture3D(volumeSampler, pos);
#if defined(NOISE_LAO_COMPUTE)
	//if (vectorData.a > minScalarRange / maxVectorLength  && vectorData.a < maxScalarRange / maxVectorLength)
#endif
	//if (vectorData.a > minScalarRange / maxVectorLength  && vectorData.a < maxScalarRange / maxVectorLength)
	{
		for (int i = 0; i < pointNum; i++)
		{
			vec3 pointRay = uniformSphericalDistr(i, angleNum, itrPerAngle);
			//vec3 pointRay = randomSphericalDistr(i, itrPerAngle);
			vec3 pointStart = pos;
			vec3 normal = vec3(0.0, 0.0, 1.0);
#if defined(NOISE_LAO_COMPUTE)
			normal = normalize(texture3D(licVolumeSampler, pos).rgb);
			mat3 rot = rotationMatrix(-1.0 * normal);
			pointRay = rot*pointRay;
#else
			normal = sampleGrad2(licVolumeSampler, pos);
			mat3 rot = rotationMatrix(-1.0 * normal);
			pointRay = rot*pointRay;
#endif
			float alphaAcu = 0;
			float curAcu = 1.0;
			vec3 pointPos = pointStart + pointRay*0.005;
			for (int k = 0; k < sampleNum; k++)
			{
				// if noise data, intensity is in a, licvolume intensity is in r
#if defined(NOISE_LAO_COMPUTE)
				float intensity = texture3D(licVolumeSampler, pointPos).a;
#else
				float intensity = texture3D(licVolumeSampler, pointPos).r;
#endif
				float alpha = texture1D(transferAlphaOpacSampler, intensity*1.3).a;
				alphaAcu = alphaAcu*(1 - alpha) + alpha;
				if (alphaAcu >= 0.9995)
					break;
				//curAcu *= 1-alpha;
				//alphaAcu += curAcu;
				//pointPos += pointRay*laoStepSize;
			}
			aoKsum += alphaAcu;
			//aoKsum += alphaAcu / sampleNum;
		}
		if (pointNum > 0)
			aoKsum /= pointNum;
		illum.rgb = vec3(aoKsum);
	}
	gl_FragColor = illum;
	//gl_FragColor = vec4(pos, 1);
}