
#ifndef _RENDERER_H_
#define _RENDERER_H_

#ifdef _WIN32
#  include <windows.h>
#endif

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "dataSet.h"
#include "mmath.h"
#include "slicing.h"
#include "texture.h"
#include "GLSLShader.h"
#include "transform.h"
#include "camera.h"
#include "types.h"
#include "VolumeBuffer.h"
#include <string>



class Renderer
{
public:
	Renderer(void);
	~Renderer(void);

	void init(char *defines = NULL);
	void resize(int w, int h);

	void render(bool update = true);

	bool saveFrameBuffer(const char *fileName);
	static bool saveTexture(const char *fileName, Texture *tex,
		const int channel = 4, const int channelMask = 15,
		const float scale = 1.0f);

	void setTechnique(RenderTechnique rt) { _renderMode = rt; }
	RenderTechnique getTechnique(void) { return _renderMode; }
	//void setIllumModel(IllumModel model) { _illumModel = model; }

	void setCamera(Camera *cam) { _cam = cam; }
	void setLight(Transform *light) { _light = light; }
	void updateLightPos(void);
	void renderLight(bool highlight = false);

	// set volume data (includes extent, center, scaling, ...)
	void setVolumeData(VolumeData *vd) { _vd = vd; }
	void setLICFilter(LICFilter *filter)
	{
		_licFilter = filter;
		_licKernelTex = _licFilter->getTextureRef();
	}

	void setClipPlanes(ClipPlane *clip, const int numPlanes)
	{
		_clipPlanes = clip;
		_numClipPlanes = numPlanes;
	}

	// toggle update scene flag
	void enableFrameStore(bool enable) { 
		_storeFrame = enable; 
		if (_storeFrame)
			std::cout << "Begin store frame." << std::endl;
		else
			std::cout << "End store frame." << std::endl;
	}
	bool isFrameStoreEnable(void) { return _storeFrame; }

	// enable FBO usage
	void enableFBO(bool enable) { _useFBO = enable; }
	bool isFBOenabled(void) { return _useFBO; }

	// the rendering resolution is halved if enable == true
	void enableLowRes(bool enable);
	bool isLowResEnabled(void) { return _lowRes; }

	void enableDebugMode(bool enable) { _debug = enable; }
	bool isDebugModeEnabled(void) { return _debug; }

	void setDataTex(Texture *tex) { _dataTex = tex; }
	void setNextDataTex(Texture *tex) { _nextDataTex = tex; }
	void setScalarTex(Texture *tex) { _scalarTex = tex; }
	void setNoiseTex(Texture *tex) { _noiseTex = tex; }
	void setLICFilterTex(Texture *tex) { _licKernelTex = tex; }
	void setLambda2Tex(Texture *tex) { _lambda2Tex = tex; }
	void setTFrgbTex(Texture *tex) { _tfRGBTex = tex; }
	void setTFalphaOpacTex(Texture *tex) { _tfAlphaOpacTex = tex; }
	void setIllumZoecklerTex(Texture *tex) { _illumZoecklerTex = tex; }
	void setIllumMalloDiffTex(Texture *tex) { _illumMalloDiffTex = tex; }
	void setIllumMalloSpecTex(Texture *tex) { _illumMalloSpecTex = tex; }

	void setWireframe(bool enable) { _wireframe = enable; }
	void screenshot(void) { _screenShot = true; }
	void switchRecording(void) { _recording = !_recording; }

	void setLICParams(LICParams *params) { _licParams = params; }
	/*
	void setStepSize(float stepSize) { _stepSize = stepSize; }
	void setGradientScale(float scale) { _gradientScale = scale; }
	void setIllumScale(float scale) { _illumScale = scale; }
	void setFreqScale(float scale) { _freqScale = scale; }
	void setTexScale(float scale) { _texScale = scale; }
	void setNumIterations(int iter) { _numIterations = iter; }
	void setLICSteps(...)
	*/

	// load glsl shader from files
	void loadGLSLShader(char *defines = NULL);
	void drawCubeFaces(void);
	void drawXYZAixs(void);

	// update slicing parameter
	void updateSlices(void);

	// update3D LIC Volume
	void updateLICVolume(void);
	// Using FBO calculate 3D LIC value and store them into a 3D Texture
	void renderLICVolume(void);
	// restore Old LIC Volume
	void restoreLICVolume(void);

	void setAnimationFlag(bool flag) { _isAnimationOn = flag; }
protected:
	void createFBO(void);
	// adapt framebuffer objects to new resolution
	void updateFBO(void);

	// adapt the offset texture to new resolution
	void updateMCOffsetTex(int width, int height);


	// draw the bounding box faces of the volume
	// draw the bounding box of the volume
	void drawCubeWireframe(void);

	void enableClipPlanes(void);
	void disableClipPlanes(void);

	void setRenderVolParams(GLSLParamsLIC *param);
	void setRenderVolTextures(GLSLParamsLIC *param);

	// render only the volume
	void renderVolume(void);

	// performs ray casting
	void raycastVolume(void);

	// performs slicing
	void sliceVolume(void);

	// fills the hole in the clipped cube
	void drawClippedPolygon(void);


	// Using Volume Rendering to render LIC 3D volume
	void raycastLICVolume(void);

	// draw a screen filling quad and display FBO content 
	// composited with a background color
	void renderBackground(void);


	inline void vertexf(const float &x, const float &y, const float &z)
	{
		glMultiTexCoord3fARB(GL_TEXTURE0_ARB, x, y, z);
		glVertex3f(x, y, z);
	}

	inline void vertexfv(const float *v)
	{
		glMultiTexCoord3fvARB(GL_TEXTURE0_ARB, v);
		glVertex3fv(v);
	}

	inline void vertexd(const double &x, const double &y, const double &z)
	{
		glMultiTexCoord3dARB(GL_TEXTURE0_ARB, x, y, z);
		glVertex3d(x, y, z);
	}

	inline void vertexdv(const double *v)
	{
		glMultiTexCoord3dvARB(GL_TEXTURE0_ARB, v);
		glVertex3dv(v);
	}

private:

	// reference to fbo object
	GLuint _framebuffer;
	// reference to depth buffer of the fbo
	GLuint _depthbuffer;
	// reference to stencil buffer of the fbo
	GLuint _stencilbuffer;

	int _winWidth;
	int _winHeight;

	int _renderWidth;
	int _renderHeight;

	bool _useFBO;

	RenderTechnique _renderMode;
	//IllumModel _illumModel;

	float _lightPos[4];

	VolumeData *_vd;
	LICFilter *_licFilter;

	// reference to clip planes
	ClipPlane *_clipPlanes;
	int _numClipPlanes;

	ClipPlane _nearClipPlane;

	// view aligned slicing 
	ViewSlicing _slices;

	// 3D LIC Volume Buffer
	VolumeBuffer * _licvolumebuffer;

	// GLSL shaders
	GLSLShader _bgShader;

	GLSLShader _raycastShader;
	GLSLShader _sliceShader;
	GLSLShader _sliceBlendShader;
	GLSLShader _volumeShader;
	GLSLShader _volumeRenderShader;
	GLSLShader _licRaycastShader;

	GLSLShader _phongShader;

	// memory locations of uniforms
	GLSLParamsBackground _paramBackground;
	GLSLParamsLIC _paramRaycast;
	GLSLParamsLIC _paramSlice;
	GLSLParamsLIC _paramSliceBlend;
	GLSLParamsLIC _paramVolume;
	GLSLParamsLIC _paramLICVolume;
	GLSLParamsLIC _paramLicRaycast;


	// Textures

	// first rendertarget
	Texture *_imgBufferTex0;
	// second rendertarget
	Texture *_imgBufferTex1;

	Texture *_mcOffsetTex;

	// vector data
	Texture *_dataTex;
	Texture *_nextDataTex;
	Texture *_scalarTex;
	Texture *_noiseTex;
	// LIC filter kernel
	Texture *_licKernelTex;
	// lambda2 field
	Texture *_lambda2Tex;
	// transfer function
	Texture *_tfRGBTex;
	Texture *_tfAlphaOpacTex;
	// illumination
	Texture *_illumZoecklerTex;
	Texture *_illumMalloDiffTex;
	Texture *_illumMalloSpecTex;

	// quadric for cylinder and a disk
	GLUquadricObj *_quadric;

	// reference to scene cam
	Camera *_cam;
	// reference to scene light
	Transform *_light;

	bool _storeFrame;
	bool _lowRes;
	//bool _requestHighRes;

	bool _wireframe;
	bool _screenShot;
	bool _recording;
	std::string _snapshotFileName;
	bool _isAnimationOn;
	int frames;


	LICParams *_licParams;

	bool _debug;
};


#endif // _RENDERER_H_
