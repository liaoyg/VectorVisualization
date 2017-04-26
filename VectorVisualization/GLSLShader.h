
#ifndef _GLSLSHADER_H_
#define _GLSLSHADER_H_

#ifdef _WIN32
#  include <windows.h>
#endif

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>

// use this to avoid a memory alignment bug in NVIDIA's glShaderSourceARB implementation 
#define NVIDIA_glShaderSourceARB_BUG(x) (((x)/4 + 1)*4)


class GLSLShader
{
public:
    GLSLShader(void);
    ~GLSLShader(void);

    // load GLSL shader from various files.
    //   defines have to be in their final form, i.e. '#define name'
    //   multiple defines are separated by '\n'
    bool loadShader(int countVS, char **vertexShaderNames, 
                    int countFS, char **fragmentShaderNames,
                    char *defines=NULL);

    bool isInitialized(void) { return _initialized; }

    inline void enableShader(void) { glUseProgramObjectARB(_programObj); }
    static inline void disableShader(void) { glUseProgramObjectARB(0); }

    GLhandleARB getProgramObj(void) { return _programObj; }
    GLhandleARB getVertexShaderObj(void) { return _vertexShaderObj; }
    GLhandleARB getFragmentShaderObj(void) { return _fragmentShaderObj; }
    
    void printInfoLog(FILE *file, GLhandleARB object);
    void printInfoLog(std::ostream &stream, GLhandleARB object);

protected:
    char* loadSource(char *fileName, int *size=NULL);

private:
    GLhandleARB _programObj;
    GLhandleARB _vertexShaderObj;
    GLhandleARB _fragmentShaderObj;    

    bool _initialized;
    
    bool _vertShaderAttached;
    bool _fragShaderAttached;
};


// structs storing the memory location of uniform glsl parameters
struct GLSLParamsLIC
{
    GLSLParamsLIC(void);
    ~GLSLParamsLIC(void) {}

    void getMemoryLocations(GLhandleARB programObj, bool printList=false);

    GLint viewport;
    GLint texMax;
    GLint scaleVol;
    GLint scaleVolInv;
    GLint stepSize;
    GLint gradient;
    GLint licParams;
    GLint licKernel;
    GLint numIterations;
    GLint alphaCorrection;

    GLint volumeSampler;
	GLint scalarSampler;
    GLint noiseSampler;
    GLint mcOffsetSampler;
    GLint transferRGBASampler;
    GLint transferAlphaOpacSampler;
    GLint licKernelSampler;
    GLint malloDiffSampler;
    GLint malloSpecSampler;
    GLint zoecklerSampler;
	GLint licVolumeSampler;
	GLint licVolumeSamplerOld;

    GLint imageFBOSampler;

	GLint interpSize;
	GLint interpStep;
};

struct GLSLParamsBackground
{
    GLSLParamsBackground(void);
    ~GLSLParamsBackground(void) {}

    void getMemoryLocations(GLhandleARB programObj, bool printList=false);
        
    GLint viewport;
    GLint bgColor;
    GLint imageFBOSampler;
};

#endif // _GLSLSHADER_H_
