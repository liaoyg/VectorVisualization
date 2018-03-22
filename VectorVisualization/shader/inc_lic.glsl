vec3 frequency(in vec3 objPos, in float freqScale,
               out float logEyeDist)
{
    float eyeDist = dot(vec4(gl_ModelViewProjectionMatrix[0][2],
                             gl_ModelViewProjectionMatrix[1][2],
                             gl_ModelViewProjectionMatrix[2][2],
                             gl_ModelViewProjectionMatrix[3][2]), 
                        vec4(objPos, 1.0));
    logEyeDist = log2(eyeDist + 1.0);

    vec2 freqMeasure = vec2(-floor(logEyeDist));
    float freqRemainder = logEyeDist + freqMeasure.x;
    //  remainder is needed for
    //  LRP illum, freqMeasureFrac.w, noise2, noise1;

    freqMeasure.x = exp2(freqMeasure.x);
    freqMeasure.y = exp2(freqMeasure.y - 1.0);
	
    freqMeasure *= freqScale;

    return vec3(freqMeasure, freqRemainder);
}


vec4 noiseLookupGrad(in vec3 objPos, 
                     in float freqScale, 
                     out float logEyeDist)
{
    vec3 freqMeasure = frequency(objPos, freqScale, logEyeDist);

    vec3 freqTexCoord = objPos * freqMeasure.x;
    vec4 noise1 = texture3D(noiseSampler, freqTexCoord);
    freqTexCoord = objPos * freqMeasure.y;
    vec4 noise2 = texture3D(noiseSampler, freqTexCoord);

    // scale gradients from [0,1] to [-1,1]
    noise1.rgb = 2.0*noise1.rgb - 1.0;
    noise2.rgb = 2.0*noise2.rgb - 1.0;

    // perform linear interpolation
    return mix(noise1, noise2, freqMeasure.z);
}


float noiseLookup(in vec3 objPos, in float freqScale, out float logEyeDist)
{
    vec3 freqMeasure = frequency(objPos, freqScale, logEyeDist);

    vec3 freqTexCoord = objPos * freqMeasure.x;
    float noise1 = texture3D(noiseSampler, freqTexCoord).a;
    freqTexCoord = objPos * freqMeasure.y;
    float noise2 = texture3D(noiseSampler, freqTexCoord).a;

    // perform linear interpolation
    return mix(noise1, noise2, freqMeasure.z);
}

// preserve same spatial frequencies of the noise with respect to
// the image plane
vec4 freqSamplingGrad(in vec3 pos, in int level, out float logEyeDist, in sampler3D vectorVolume = volumeSampler)
{
	vec4 vectorData = texture3D(vectorVolume, pos);

	//float minRange = 0.0;
	//float maxRange = 30.0;
	//magnitude range equation
	if (vectorData.a > minScalarRange / maxVectorLength  && vectorData.a < maxScalarRange / maxVectorLength)
	{
		//return texture3D(noiseSampler, pos).a
		vec4 res = texture3D(noiseSampler, pos*gradient.z);
		if (level > 0)
			return res.a > 0.0 ? texture3D(noiseSampler, pos*level*gradient.z) : vec4(0.0);
		else
			return res;
		//return noiseLookup(pos, gradient.z, logEyeDist);
	}
	else
		return vec4(0.0);
    //return noiseLookupGrad(pos, gradient.z, logEyeDist);
}


float freqSampling(in vec3 pos, in int level, out float logEyeDist, in sampler3D vectorVolume = volumeSampler)
{
    //vec3 objPos = pos * scaleVolInv.xyz;

	//Use scalar data to decide noise range to be integrated
	vec4 vectorData = texture3D(vectorVolume, pos);
	//vec4 scalarData = texture3D(scalarSampler, pos); 
	//float scala = length(vectorData.xyz);
	
	//if (scalarData.r > 0.1  && scalarData.r < 0.3)
	
	// one example of visualize range: magnetic_field (magnitude 7.0 ~ max)
	//float minRange = 2.0;
	//float maxRange = 30.0;
	//magnitude range equation
	//minScalarRange = 1.0;
	//maxScalarRange = 13.0;
	if (vectorData.a > minScalarRange /maxVectorLength  && vectorData.a < maxScalarRange /maxVectorLength)
	// g component range equation: map to 0 ~ 1.0, 0 is 0.5
	//if (vectorData.b > ( 0.5 * minRange/maxVectorLength + 0.5)  && vectorData.b < ( 0.5 * maxRange/maxVectorLength + 0.5))
	//if (true)
	{
		//return texture3D(noiseSampler, pos).a
		float res = texture3D(noiseSampler, pos*gradient.z).a;
		if(level > 0)
			return res > 0.0 ? texture3D(noiseSampler, pos*level*gradient.z).a : 0.0;
		else 
			return res;
		//return noiseLookup(pos, gradient.z, logEyeDist);
	}
	else
		return 0;
}

float noiseLAOSampling(in vec3 pos, in int level, in sampler3D vectorVolume = volumeSampler)
{
	//Use scalar data to decide noise range to be integrated
	vec4 vectorData = texture3D(vectorVolume, pos);

	// one example of visualize range: magnetic_field (magnitude 7.0 ~ max)
	//float minRange = 2.0;
	//float maxRange = 30.0;
	//magnitude range equation
	if (vectorData.a > minScalarRange / maxVectorLength  && vectorData.a < maxScalarRange / maxVectorLength)
	{
		//return texture3D(noiseSampler, pos).a
		float res = texture3D(noiseLAOSampler, pos*gradient.z).r;
		if (level > 0)
			return res > 0.0 ? texture3D(noiseLAOSampler, pos*level*gradient.z).r : 0.0;
		else
			return res;
		//return noiseLookup(pos, gradient.z, logEyeDist);
	}
	else
		return 0;
}

#ifdef USE_NOISE_GRADIENTS
vec4 singleLICstep(in vec3 licdir, in out vec3 newPos, in int level,
                   in out vec4 step, in float kernelOffset,
                   in out float logEyeDist, in float dir, in out float ao,
				   in sampler3D vectorVolume  = volumeSampler)
{
    vec4 noise;
#else
float singleLICstep(in vec3 licdir, in out vec3 newPos, in int level,
                   in out vec4 step, in float kernelOffset,
                   in out float logEyeDist, in float dir, in out float ao,
				   in sampler3D vectorVolume  = volumeSampler)
{
    float noise;
#endif

    // consider speed of flow
#ifdef SPEED_OF_FLOW
    licdir *= step.a;
#endif

    // scale with LIC step size
    // also correct length according to camera distance
    licdir *= licParams.z * (logEyeDist*0.5 + 0.3);
    vec3 Pos2 = newPos + licdir;
	vec4 step2 = texture3D(vectorVolume, Pos2);
	vec3 licdir2 = 2.0*step2.rgb - 1.0;
	licdir2 *= dir;
	//licdir2 = step2.rgb;
#ifdef SPEED_OF_FLOW
    licdir2 *= step.a;
#endif
	licdir2 *= licParams.z * (logEyeDist*0.5 + 0.3);
	//Pos2 += 0.5 * licdir2;
	newPos += 0.5 * (licdir + licdir2);
	//newPos += 0.3 * licdir;

    step = texture3D(vectorVolume, newPos);
#ifdef TIME_DEPENDENT
    vectorFieldSample2 = texture3D(volumeSampler2, newPos);
    step = mix(timeStep, step, vectorFieldSample2);
#endif

#ifdef USE_NOISE_GRADIENTS
    noise = freqSamplingGrad(newPos, level, logEyeDist);
    //noise = vec4(texture3D(noiseSampler, newPos).a);
#else
    noise = freqSampling(newPos, level, logEyeDist);
#endif

    // determine weighting
    noise *= texture1D(licKernelSampler, kernelOffset).r;
#ifdef AMBIENT_OCCULUSION
	ao += noiseLAOSampling(newPos, level) * texture1D(licKernelSampler, kernelOffset).r;
#endif


    return noise;
}



// performs the LIC computation for n steps forward and backward
//    pos determines the starting position of the LIC
//    vectorFieldSample is the value of the vector field at this position
vec4 computeLIC(in vec3 pos, in vec4 vectorFieldSample, 
	out vec2 streamDis, out vec3 streamStart, 
	out vec3 streaEnd,
	in sampler3D vectorVolume = volumeSampler)
{
    vec3 licdir;
    float logEyeDist;
    float kernelOffset = 0.5;
	float ao = 1.0;
#ifdef TIME_DEPENDENT
    vec4 vectorFieldSample2;
#endif
	// decide which noise sampling level will use for this lic computation
	int level = 0;
	//if (pos.z > 0.5)
	//	level = 2;
	//else
	//	level = 1;
	
    // perform first lookup
#ifdef USE_NOISE_GRADIENTS
    vec4 noise;
    vec4 illum = freqSamplingGrad(pos,level, logEyeDist);
#else
    float noise;
    float illum = freqSampling(pos, level, logEyeDist);
#endif

    // weight sample with lic kernel at position 0
    illum *= texture1D(licKernelSampler, 0.5).r;
#ifdef AMBIENT_OCCULUSION
	ao = noiseLAOSampling(pos, level) * texture1D(licKernelSampler, 0.5).r;
#endif   
	float dir = -1;
    // backward LIC
    vec3 newPos = pos;
    vec4 step = vectorFieldSample;
	float streamlineL = 0.0;
	float sumCross = 0.0;
    for (int i=0; i<int(licParams.y); ++i)
    {
        licdir = -2.0*step.rgb + 1.0;
		//licdir = step.rgb;
        kernelOffset -= licKernel.y;
		vec3 oldPos = newPos;
        illum += singleLICstep(licdir, newPos, level, step, 
                               kernelOffset, logEyeDist, dir, ao, vectorVolume);
		streamlineL += length(newPos - oldPos);
		sumCross += length(cross(oldPos, newPos));
    }
	streaEnd = newPos;
    // forward LIC
	dir = 1;
    newPos = pos;
    step = vectorFieldSample;
    kernelOffset = 0.5;
    for (int i=0; i<int(licParams.x); ++i)
    {
        licdir = 2.0*step.rgb - 1.0;
		//licdir = step.rgb;
        kernelOffset += licKernel.x;
		vec3 oldPos = newPos;
        illum += singleLICstep(licdir, newPos, level, step, 
                               kernelOffset, logEyeDist, dir, ao, vectorVolume);
		streamlineL += length(newPos - oldPos);
		sumCross += length(cross(newPos, oldPos));
    }
    streamStart = newPos;
	streamDis = vec2(streamlineL, sumCross);
    return vec4(illum);
}

vec4 computeLICwithAO(in vec3 pos, in vec4 vectorFieldSample,
	out vec2 streamDis, out vec3 streamStart,
	out vec3 streaEnd, in out float ao,
	in sampler3D vectorVolume = volumeSampler)
{
	vec3 licdir;
	float logEyeDist;
	float kernelOffset = 0.5;
#ifdef TIME_DEPENDENT
	vec4 vectorFieldSample2;
#endif
	// decide which noise sampling level will use for this lic computation
	int level = 0;
	//if (pos.z > 0.5)
	//	level = 2;
	//else
	//	level = 1;

	// perform first lookup
#ifdef USE_NOISE_GRADIENTS
	vec4 noise;
	vec4 illum = freqSamplingGrad(pos, level, logEyeDist);
#else
	float noise;
	float illum = freqSampling(pos, level, logEyeDist);
#endif
	//return vec4(illum);

	// weight sample with lic kernel at position 0
	illum *= texture1D(licKernelSampler, 0.5).r;
#ifdef AMBIENT_OCCULUSION
	ao = noiseLAOSampling(pos, level) * texture1D(licKernelSampler, 0.5).r;
#endif   
		float dir = -1;
	// backward LIC
	vec3 newPos = pos;
	vec4 step = vectorFieldSample;
	float streamlineL = 0.0;
	float sumCross = 0.0;
	for (int i = 0; i<int(licParams.y); ++i)
	{
		licdir = -2.0*step.rgb + 1.0;
		//licdir = step.rgb;
		kernelOffset -= licKernel.y;
		vec3 oldPos = newPos;
		illum += singleLICstep(licdir, newPos, level, step,
			kernelOffset, logEyeDist, dir, ao, vectorVolume);
		streamlineL += length(newPos - oldPos);
		sumCross += length(cross(oldPos, newPos));
	}
	streaEnd = newPos;
	// forward LIC
	dir = 1;
	newPos = pos;
	step = vectorFieldSample;
	kernelOffset = 0.5;
	for (int i = 0; i<int(licParams.x); ++i)
	{
		licdir = 2.0*step.rgb - 1.0;
		//licdir = step.rgb;
		kernelOffset += licKernel.x;
		vec3 oldPos = newPos;
		illum += singleLICstep(licdir, newPos, level, step,
			kernelOffset, logEyeDist, dir, ao, vectorVolume);
		streamlineL += length(newPos - oldPos);
		sumCross += length(cross(newPos, oldPos));
	}
	streamStart = newPos;
	streamDis = vec2(streamlineL, sumCross);
	return vec4(illum);
}

float computeStreamlineDis(in vec3 p, in vec3 q )
{
	vec4 pVector = texture3D(volumeSampler, p);
	vec4 qVector = texture3D(volumeSampler, q);
	int itrNum = 32;
	float area = 0.0;
	float lengthp = 0.0;
	float lengthq = 0.0;

	vec3 oldp = p;
	vec3 oldq = q;
	//backward
	for(int i = 0; i < itrNum; i++)
	{
		vec3 pDir = (-2.0*pVector.rgb + 1.0)*licParams.z * 0.3;
		vec3 qDir = (-2.0*qVector.rgb + 1.0)*licParams.z * 0.3;
		vec3 newp = oldp+pDir;
		vec3 newq = oldq+qDir;
		pVector = texture3D(volumeSampler, newp);
		qVector = texture3D(volumeSampler, newp);
		vec3 pDir2 = (-2.0*pVector.rgb + 1.0)*licParams.z * 0.3;
		vec3 qDir2 = (-2.0*qVector.rgb + 1.0)*licParams.z * 0.3;
		newp = oldp+0.5*(pDir+pDir2);
		newq = oldq+0.5*(qDir+qDir2);

		area += 0.5*(length(cross(oldq-oldp,newq-oldp)) + length(cross(oldp-newq, newp-newq)));
		lengthp += length(0.5*(pDir+pDir2));
		lengthq += length(0.5*(qDir+qDir2));
		oldp = newp;
		oldq = newq;
		pVector = texture3D(volumeSampler, oldp);
		qVector = texture3D(volumeSampler, oldq);
	}

	//forward
	pVector = texture3D(volumeSampler, p);
	qVector = texture3D(volumeSampler, q);
	oldp = p;
	oldq = q;
	for(int i = 0; i < itrNum; i++)
	{
		vec3 pDir = (2.0*pVector.rgb - 1.0)*licParams.z * 0.3;
		vec3 qDir = (2.0*qVector.rgb - 1.0)*licParams.z * 0.3;
		vec3 newp = oldp+pDir;
		vec3 newq = oldq+qDir;
		pVector = texture3D(volumeSampler, newp);
		qVector = texture3D(volumeSampler, newp);
		vec3 pDir2 = (2.0*pVector.rgb - 1.0)*licParams.z * 0.3;
		vec3 qDir2 = (2.0*qVector.rgb - 1.0)*licParams.z * 0.3;
		newp = oldp+0.5*(pDir+pDir2);
		newq = oldq+0.5*(qDir+qDir2);

		area += 0.5*(length(cross(oldq-oldp,newq-oldp)) + length(cross(oldp-newq, newp-newq)));
		lengthp += length(0.5*(pDir+pDir2));
		lengthq += length(0.5*(qDir+qDir2));
		oldp = newp;
		oldq = newq;
		pVector = texture3D(volumeSampler, oldp);
		qVector = texture3D(volumeSampler, oldq);
	}

	return 2.0*area/(lengthp+lengthq);
}


