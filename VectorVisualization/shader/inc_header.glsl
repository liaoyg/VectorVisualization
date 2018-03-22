#extension GL_ARB_texture_rectangle : enable

// define either ILLUM_GRADIENT, ILLUM_MALLO, ILLUM_ZOECKLER, or ILLUM_NO
// if nothing is defined ILLUM_NO is used

//#define ILLUM_ZOECKLER
//#define ILLUM_MALLO
//#define ILLUM_GRADIENT


//#define SPEED_OF_FLOW
//#define TIME_DEPENDENT
//#define USE_NOISE_GRADIENTS
//#define USE_MC_OFFSET


#if defined(ILLUM_GRADIENT)
#   define USE_NOISE_GRADIENTS
#endif


// max texture coords for bounding box test
uniform vec4 texMax;

// scale factors of the volume
uniform vec4 scaleVol;
uniform vec4 scaleVolInv;

uniform float stepSize;
uniform vec3 gradient; // scale, illum scale, frequency scale

uniform int numIterations;

uniform float alphaCorrection;

uniform vec3 licParams;  // lics steps forward, backward, lic step width

uniform vec3 licKernel;  // kernel step width forward (0.5/licParams.x),
                         // kernel step width backward (0.5/licParams.y),
                         // inverse filter area

uniform float timeStep;

uniform float maxVectorLength;

 float minScalarRange = 4.0;
 float maxScalarRange = 14.0;


// textures (have to be uniform)
uniform sampler3D volumeSampler;
uniform sampler3D volumeSamplerNext;
uniform sampler3D scalarSampler;
uniform sampler3D noiseSampler;
uniform sampler3D noiseLAOSampler;

uniform sampler2DRect mcOffsetSampler;

uniform sampler1D transferRGBASampler;
uniform sampler1D transferAlphaOpacSampler;

uniform sampler1D licKernelSampler;


uniform sampler2D malloDiffSampler;
uniform sampler2D malloSpecSampler;
uniform sampler2D zoecklerSampler;

uniform sampler2DRect imageFBOSampler;


