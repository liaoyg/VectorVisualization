#ifdef _WIN32
#include <windows.h>
#endif

#define _USE_MATH_DEFINES

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL\glew.h>
//#include <GL\freeglut.h>
//#include <GL/gl.h>
//#include <GL/glu.h>

#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "dataSet.h"
#include "mmath.h"
#include "imageUtils.h"
#include "texture.h"
#include "GLSLShader.h"
#include "transform.h"
#include "camera.h"
#include "types.h"
#include "renderer.h"


Renderer::Renderer(void) : _framebuffer(0), _depthbuffer(0), _stencilbuffer(0),
_winWidth(1), _winHeight(1), _useFBO(false),
_renderMode(VOLIC_RAYCAST), _vd(NULL), _licFilter(NULL),
_dataTex(NULL), _noiseTex(NULL), _licKernelTex(NULL), _scalarTex(NULL),
_lambda2Tex(NULL), _tfRGBTex(NULL), _tfAlphaOpacTex(NULL),
_illumZoecklerTex(NULL), _illumMalloDiffTex(NULL),
_illumMalloSpecTex(NULL), _quadric(NULL), _storeFrame(true),
_lowRes(false), _wireframe(false), _screenShot(false), _recording(false), _licParams(NULL),
_debug(false), _isAnimationOn(false)

{
	_imgBufferTex0 = new Texture;
	_imgBufferTex1 = new Texture;
	_mcOffsetTex = new Texture;

	_nearClipPlane.setActive(true);

	_snapshotFileName = "snapshot.png";
	frames = 0;

}


Renderer::~Renderer(void)
{
	// unbind fbo
	if (_framebuffer && _depthbuffer
		&& glFramebufferTexture2DEXT && glBindFramebufferEXT
		&& glDeleteFramebuffersEXT && glDeleteRenderbuffersEXT)
	{
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_RECTANGLE_ARB,
			0, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		glDeleteFramebuffersEXT(1, &_framebuffer);
		glDeleteRenderbuffersEXT(1, &_depthbuffer);
	}
	glDeleteTextures(1, &_imgBufferTex0->id);
	glDeleteTextures(1, &_imgBufferTex1->id);

	delete _imgBufferTex0;
	delete _imgBufferTex1;
	delete _mcOffsetTex;

	if (_quadric)
		gluDeleteQuadric(_quadric);
}


void Renderer::init(char *defines)
{
	float lightPos[] = { 0.0f, 0.0, 0.0f, 0.0f };

	_quadric = gluNewQuadric();
	gluQuadricDrawStyle(_quadric, GLU_FILL);
	gluQuadricNormals(_quadric, GLU_SMOOTH);
	CHECK_FOR_OGL_ERROR();

	createFBO();
	CHECK_FOR_OGL_ERROR();

	//init volume buffer
	// A 3D texture buffer to store LIC value according to the vectore field
	_licvolumebuffer = new VolumeBuffer(GL_RGBA16F_ARB, 512, 512, 512, 2);

	loadGLSLShader(defines);
	CHECK_FOR_OGL_ERROR();

	glLightfv(GL_LIGHT1, GL_POSITION, lightPos);
}


void Renderer::resize(int w, int h)
{
	_winWidth = w;
	_winHeight = h;

	if (_lowRes)
	{
		_renderWidth = MAX(_winWidth / 2, 1);
		_renderHeight = MAX(_winHeight / 2, 1);
	}
	else
	{
		_renderWidth = _winWidth;
		_renderHeight = _winHeight;
	}

	updateFBO();
}


void Renderer::render(bool update)
{
	Vector3 dir = Vector3_new(0.0, 0.0, 1.0);
	Quaternion q_camInv = Quaternion_inverse(_cam->getQuaternion());

	CHECK_FOR_OGL_ERROR();

	_cam->setCamera();
	CHECK_FOR_OGL_ERROR();

	// TODO: animation?
	// glRotatef(g.animatedAngle, 0.0, 1.0, 0.0);

	//updateLightPos();

	dir = Quaternion_multVector3(q_camInv, dir);

	_nearClipPlane.setNormal(-dir.x, -dir.y, -dir.z,
		_cam->getDistance() - _cam->getPosition().z - (NEAR_CLIP + 0.005));

	glTranslatef(-_vd->center[0], -_vd->center[1], -_vd->center[2]);

	CHECK_FOR_OGL_ERROR();

	// only redraw complete scene into FBO when necessary
	// use previous result otherwise 
	if (update)
	{
		// update viewport to render width and heigth
		// when using low resolution rendering
		//_lowRes = true;
		if (_lowRes)
			glViewport(0, 0, _renderWidth, _renderHeight);

		glPushMatrix();
		glTranslatef(_vd->center[0], _vd->center[1], _vd->center[2]);
		enableClipPlanes();
		glPopMatrix();

		// bind fbo texture
		if (_useFBO)
		{
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _framebuffer);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT,
				_imgBufferTex0->texTarget,
				_imgBufferTex0->id, 0);
			if (_renderMode != VOLIC_SLICING)
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			CHECK_FRAMEBUFFER_STATUS();
			CHECK_FOR_OGL_ERROR();
		}

		switch (_renderMode)
		{
		case VOLIC_VOLUME:
			renderVolume();
			break;
		case VOLIC_RAYCAST:
			raycastVolume();
			break;
		case VOLIC_SLICING:
			sliceVolume();
			break;
		case VOLIC_LICVOLUME:
			raycastLICVolume();
			break;
		default:
			std::cerr << "Renderer:  Unknown render mode!" << std::endl;
			break;
		}
		CHECK_FOR_OGL_ERROR();

		disableClipPlanes();

		drawXYZAixs();
		CHECK_FOR_OGL_ERROR();

		// unbind fbo texture
		if (_useFBO)
		{
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT,
				GL_TEXTURE_RECTANGLE_ARB,
				0, 0);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			CHECK_FRAMEBUFFER_STATUS();
			CHECK_FOR_OGL_ERROR();
		}
		// read back content of framebuffer when not using FBOs
		else if (_storeFrame || _screenShot || _recording)
		{
			//glReadBuffer(GL_BACK);
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(_imgBufferTex0->texTarget);
			glBindTexture(_imgBufferTex0->texTarget, _imgBufferTex0->id);
			glCopyTexImage2D(_imgBufferTex0->texTarget, 0, GL_RGBA,
				0, 0, _winWidth, _winHeight, 0);
			glDisable(_imgBufferTex0->texTarget);
			CHECK_FOR_OGL_ERROR();
		}
	}
	if (_useFBO || _storeFrame || _screenShot || _recording)
		renderBackground();

	GLSLShader::disableShader();
	CHECK_FOR_OGL_ERROR();


	// TODO:   adapt it for rendering an animation of single frame
	/*
	if (g.videoSequence) // && (g.frameNumber > 115))
	{
	tex.target = GL_TEXTURE_RECTANGLE_ARB;
	tex.id = g.imgBufferTex0->id;

	//    fprintf(stderr, "Writing frame %05d ... ", g.frameNumber);

	snprintf(imgFileName, 100, "video/image%05d.png", g.frameNumber);
	writeImage(imgFileName, &tex, g.windowWidth, g.windowHeight);

	//    fprintf(stderr, "done.\n");

	g.frameNumber++;

	//    if (((g.frameNumber > 299) && (g.animRotation == 0)) || (g.frameNumber > 359))
	if (((g.frameNumber > 359) && (g.vMode == VIDEO_ROTATION))
	|| ((g.frameNumber > 299) && (g.vMode == VIDEO_DOLLY))
	|| ((g.frameNumber > 499) && (g.vMode == VIDEO_TIMEDEP)))
	{
	// we're done. disable frame writing
	fprintf(stdout, "Video rendering complete (%d frames rendered)\n",
	g.frameNumber);
	g.videoSequence = 0;
	g.animated = 0;
	//      g.animRotation = 0;
	g.vMode = VIDEO_NONE;
	g.updateFrameBufferTex = 1;
	g.u.translate[0] = g.videoObjectDist[0];
	g.u.translate[1] = g.videoObjectDist[1];
	g.u.translate[2] = g.videoObjectDist[2];
	}
	}
	*/

	glPushMatrix();
	glTranslatef(_vd->center[0], _vd->center[1], _vd->center[2]);
	enableClipPlanes();
	glPopMatrix();

	if (_wireframe)
	{
		drawCubeWireframe();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		for (int i = 0; i<_numClipPlanes; ++i)
			_clipPlanes[i].drawSlice();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	CHECK_FOR_OGL_ERROR();

	


	// write depth values of the bounding box into the framebuffer
	/*
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	drawQuads();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	*/

	// disable clip planes and draw the activated ones
	disableClipPlanes();

	// draw clip planes
	CHECK_FOR_OGL_ERROR();

	glPushMatrix();
	glTranslatef(_vd->center[0], _vd->center[1], _vd->center[2]);
	glDisable(GL_DEPTH_TEST);
	for (int i = 0; i<_numClipPlanes; ++i)
		_clipPlanes[i].draw();
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();

	CHECK_FOR_OGL_ERROR();
}


bool Renderer::saveFrameBuffer(const char *fileName)
{
	Image img;
	bool succesful;

	if (!fileName)
		return false;

	img.imgData = new unsigned char[3 * _winWidth*_winHeight];
	img.width = _winWidth;
	img.height = _winHeight;
	img.channel = 3;

	glReadPixels(0, 0, _winWidth, _winHeight, GL_RGB, GL_UNSIGNED_BYTE, img.imgData);
	CHECK_FOR_OGL_ERROR();

	succesful = pngWrite(fileName, &img, true);

	delete[] img.imgData;
	img.imgData = NULL;

	return succesful;
}


bool Renderer::saveTexture(const char *fileName, Texture *tex,
	const int channel, const int channelMask,
	const float scale)
{
	Image img;
	int v;
	float *data = NULL;
	bool succesful;
	bool alpha;
	GLenum srcChannels = GL_RGBA;

	if (!fileName)
		return false;

	if ((channel < 1) || (channel > 4))
		return false;

	img.imgData = new unsigned char[channel*tex->width*tex->height];
	img.width = tex->width;
	img.height = tex->height;
	img.channel = channel;

	// handle alpha channel separately if it is not masked
	alpha = (((channel == 2) && !(channelMask & 2))
		|| ((channel == 4) && !(channelMask & 8)));

	switch (channel)
	{
	case 1:
		srcChannels = GL_RED;
		break;
	case 2:
		srcChannels = GL_LUMINANCE_ALPHA;
		break;
	case 3:
		srcChannels = GL_RGB;
		break;
	case 4:
		srcChannels = GL_RGBA;
		break;
	}

	// get texture data
	CHECK_FOR_OGL_ERROR();
	tex->bind();

	// special handling for floating point textures
	if ((tex->format == GL_RGBA16F_ARB)
		|| (tex->format == GL_RGBA32F_ARB))
	{
		data = new float[channel*tex->width*tex->height];

		glGetTexImage(tex->texTarget, 0, srcChannels, GL_FLOAT, data);

		// convert the floating point texture data to unsigned char
		for (int i = 0; i<tex->width*tex->height; ++i)
		{
			for (int k = 0; k<channel - alpha; ++k)
			{
				v = ((1 << k) & channelMask) ? (int)(scale*data[i*channel + k]) : 0;
				v = (v > 255) ? 255 : ((v < 0) ? 0 : v);

				img.imgData[i*channel + k] = (unsigned char)v;
			}
		}
	}
	else
	{
		// read texture data directly to image data
		glGetTexImage(tex->texTarget, 0, srcChannels, GL_UNSIGNED_BYTE, img.imgData);
	}
	tex->unbind();
	CHECK_FOR_OGL_ERROR();

	// fill alpha channel 
	if (alpha)
	{
		for (int i = 0; i<tex->width*tex->height; ++i)
			img.imgData[i*channel + channel - 1] = 255;
	}

	succesful = pngWrite(fileName, &img, true);

	delete[] data;
	delete[] img.imgData;
	img.imgData = NULL;

	return succesful;
}


void Renderer::updateLightPos(void)
{
	float transLight[4];
	float invModelView[16];
	float axis[3];
	Vector3 lightPos;

	lightPos = _light->getPosition();
	_cam->getAxis(axis);

	// compute inverse model view matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(_vd->center[0], _vd->center[1], _vd->center[2]);
	glRotatef(-_cam->getAngle()*180.0f / (float)M_PI, axis[0], axis[1], axis[2]);
	// glRotatef(-animatedAngle, 0.0, 1.0, 0.0);
	// TODO? glTranslatef(0.0, 0.0, cam->getDistance);
	glTranslatef(_cam->getPosition().x, _cam->getPosition().y, _cam->getPosition().z);

	glGetFloatv(GL_MODELVIEW_MATRIX, invModelView);

	transLight[0] = invModelView[0] * lightPos.x + invModelView[4] * lightPos.y
		+ invModelView[8] * lightPos.z + invModelView[12];
	transLight[1] = invModelView[1] * lightPos.x + invModelView[5] * lightPos.y
		+ invModelView[9] * lightPos.z + invModelView[13];
	transLight[2] = invModelView[2] * lightPos.x + invModelView[6] * lightPos.y
		+ invModelView[10] * lightPos.z + invModelView[14];
	transLight[3] = 1.0f; // it's a point light

						  // setup light position with identity modelview matrix
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, transLight);
	glPopMatrix();
	CHECK_FOR_OGL_ERROR();
}


void Renderer::renderLight(bool highlight)
{
	int slices = 24;
	float axis[3];
	float ambient[4] = { 0.4f, 0.4f, 0.4f, 1.0f };
	float diffuse[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float specular[4] = { 0.8f, 0.8f, 0.8f, 1.0f };

	if (highlight)
	{
		ambient[2] = 0.0f;
		diffuse[2] = 0.0f;
		glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
	}
	else
	{
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	_phongShader.enableShader();

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -_cam->getDistance());

	_light->getAxis(axis);
	glRotatef(_light->getAngle()*180.0f / (float)M_PI, axis[0], axis[1], axis[2]);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, -_light->getDistance());

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0f);

	// arrowhead
	gluCylinder(_quadric, 0.0625f, 0.0f, 0.125f, slices, 1);
	// cylinder
	glTranslatef(0.0f, 0.0f, -0.1f);
	gluCylinder(_quadric, 0.02f, 0.02f, 0.1f, slices, 1);
	// disk
	gluDisk(_quadric, 0.0f, 0.02f, slices, 1);

	glPopMatrix();

	_phongShader.disableShader();
	CHECK_FOR_OGL_ERROR();
}


void Renderer::enableLowRes(bool enable)
{
	_lowRes = enable;

	if (_lowRes)
	{
		_renderWidth = MAX(_winWidth / 2, 1);
		_renderHeight = MAX(_winHeight / 2, 1);
	}
	else
	{
		_renderWidth = _winWidth;
		_renderHeight = _winHeight;
	}
}


void Renderer::createFBO(void)
{
	GLuint texId[2];

	// framebuffer object already exists
	if (_framebuffer)
		return;

	// create fbo with depth buffer
	glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
	glGenFramebuffersEXT(1, &_framebuffer);
	glGenRenderbuffersEXT(1, &_depthbuffer);

	// create two render target textures
	glGenTextures(2, texId);
	_imgBufferTex0->setTex(GL_TEXTURE_RECTANGLE_ARB, texId[0], "FBO-Tex0");
	_imgBufferTex0->texUnit = GL_TEXTURE1_ARB;
	_imgBufferTex1->setTex(GL_TEXTURE_RECTANGLE_ARB, texId[1], "FBO-Tex1");
	_imgBufferTex1->texUnit = GL_TEXTURE1_ARB;

	CHECK_FOR_OGL_ERROR();
}


void Renderer::updateFBO(void)
{
	GLuint rendertargetFormat = GL_RGBA16F_ARB;

	CHECK_FOR_OGL_ERROR();

	// unbind render target textures
	_imgBufferTex0->unbind();
	_imgBufferTex1->unbind();

	CHECK_FOR_OGL_ERROR();

	// update first rendertarget
	glActiveTextureARB(_imgBufferTex0->texUnit);
	glBindTexture(_imgBufferTex0->texTarget, _imgBufferTex0->id);
	glTexImage2D(_imgBufferTex0->texTarget, 0, rendertargetFormat,
		_winWidth, _winHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	_imgBufferTex0->width = _winWidth;
	_imgBufferTex0->height = _winHeight;
	_imgBufferTex0->format = rendertargetFormat;

	glTexParameteri(_imgBufferTex0->texTarget,
		GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(_imgBufferTex0->texTarget,
		GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(_imgBufferTex0->texTarget,
		GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(_imgBufferTex0->texTarget,
		GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	CHECK_FOR_OGL_ERROR();

	// update second rendertarget
	glActiveTextureARB(_imgBufferTex1->texUnit);
	glBindTexture(_imgBufferTex1->texTarget, _imgBufferTex1->id);
	glTexImage2D(_imgBufferTex1->texTarget, 0, rendertargetFormat,
		_winWidth, _winHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	_imgBufferTex1->width = _winWidth;
	_imgBufferTex1->height = _winHeight;
	_imgBufferTex1->format = rendertargetFormat;

	glTexParameteri(_imgBufferTex1->texTarget,
		GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(_imgBufferTex1->texTarget,
		GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(_imgBufferTex1->texTarget,
		GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(_imgBufferTex1->texTarget,
		GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	CHECK_FOR_OGL_ERROR();


	// prepare fbo
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _framebuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
		_imgBufferTex0->texTarget, _imgBufferTex0->id, 0);
	// attach renderbuffer to framebuffer depth buffer
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _depthbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
		GL_DEPTH24_STENCIL8_EXT, _winWidth, _winHeight);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
		GL_RENDERBUFFER_EXT, _depthbuffer);

	// attach stencil buffer to framebuffer depth buffer
	//glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
	//    GL_RENDERBUFFER_EXT, _depthbuffer);
	CHECK_FOR_OGL_ERROR();
	CHECK_FRAMEBUFFER_STATUS();

	// unbind framebuffer texture and framebuffer
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
		GL_TEXTURE_RECTANGLE_ARB, 0, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	CHECK_FRAMEBUFFER_STATUS();
}


void Renderer::updateMCOffsetTex(int width, int height)
{
	GLuint texId;
	float *noise = NULL;

	if (((_mcOffsetTex->width == width)
		&& (_mcOffsetTex->height == height))
		|| (width < 1) || (height < 1))
	{
		return;  // no update needed/invalid texture size
	}

	if (_mcOffsetTex->id == 0)
	{
		glGenTextures(1, &texId);
		_mcOffsetTex->setTex(GL_TEXTURE_RECTANGLE_ARB, texId, "MC-OffsetTex");
		_mcOffsetTex->texTarget = GL_TEXTURE2_ARB;
	}

	_mcOffsetTex->width = width;
	_mcOffsetTex->height = height;

	noise = new float[width*height];

	// fill offset texture with values in [0,1]
	for (int i = 0; i<width*height; ++i)
	{
		noise[i] = (float)rand() / (float)RAND_MAX;
	}

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _mcOffsetTex->id);

	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE16F_ARB,
		width, height, 0, GL_LUMINANCE, GL_FLOAT, noise);

	// no linear filtering because texels should map 1:1 to pixels
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	CHECK_FOR_OGL_ERROR();

	delete[] noise;
}


void Renderer::drawCubeFaces(void)
{
	glBegin(GL_QUADS);
	{
		// back side
		glNormal3f(0.0f, 0.0f, -1.0f);
		glMultiTexCoord4fARB(GL_TEXTURE5_ARB, 0.0f, 0.0f, -1.0f, 0.0f);

		vertexf(0.0f, 0.0f, 0.0f);
		vertexf(0.0f, _vd->extent[1], 0.0f);
		vertexf(_vd->extent[0], _vd->extent[1], 0.0f);
		vertexf(_vd->extent[0], 0.0f, 0.0f);

		// front side
		glNormal3f(0.0f, 0.0f, 1.0f);
		glMultiTexCoord4fARB(GL_TEXTURE5_ARB, 0.0f, 0.0f, 1.0f, 0.0f);
		vertexf(0.0f, 0.0f, _vd->extent[2]);
		vertexf(_vd->extent[0], 0.0f, _vd->extent[2]);
		vertexf(_vd->extent[0], _vd->extent[1], _vd->extent[2]);
		vertexf(0.0f, _vd->extent[1], _vd->extent[2]);

		// top side
		glNormal3f(0.0f, 1.0f, 0.0f);
		glMultiTexCoord4fARB(GL_TEXTURE5_ARB, 0.0f, 1.0f, 0.0f, 0.0f);
		vertexf(0.0f, _vd->extent[1], 0.0f);
		vertexf(0.0f, _vd->extent[1], _vd->extent[2]);
		vertexf(_vd->extent[0], _vd->extent[1], _vd->extent[2]);
		vertexf(_vd->extent[0], _vd->extent[1], 0.0f);

		// bottom side
		glNormal3f(0.0f, -1.0f, 0.0f);
		glMultiTexCoord4fARB(GL_TEXTURE5_ARB, 0.0f, -1.0f, 0.0f, 0.0f);
		vertexf(0.0f, 0.0f, 0.0f);
		vertexf(_vd->extent[0], 0.0f, 0.0f);
		vertexf(_vd->extent[0], 0.0f, _vd->extent[2]);
		vertexf(0.0f, 0.0f, _vd->extent[2]);

		// left side
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glMultiTexCoord4fARB(GL_TEXTURE5_ARB, -1.0f, 0.0f, 0.0f, 0.0f);
		vertexf(0.0f, 0.0f, 0.0f);
		vertexf(0.0f, 0.0f, _vd->extent[2]);
		vertexf(0.0f, _vd->extent[1], _vd->extent[2]);
		vertexf(0.0f, _vd->extent[1], 0.0f);

		// right side
		glNormal3f(1.0f, 0.0f, 0.0f);
		glMultiTexCoord4fARB(GL_TEXTURE5_ARB, 1.0f, 0.0f, 0.0f, 0.0f);
		vertexf(_vd->extent[0], 0.0f, 0.0f);
		vertexf(_vd->extent[0], _vd->extent[1], 0.0f);
		vertexf(_vd->extent[0], _vd->extent[1], _vd->extent[2]);
		vertexf(_vd->extent[0], 0.0f, _vd->extent[2]);
	}
	glEnd();
}

void Renderer::drawXYZAixs(void)
{
	int oldViewport[4];
	glGetIntegerv(GL_VIEWPORT, oldViewport);
	glViewport(1000, 100, 100, 100);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	//gluPerspective(45.0f, 1.0f, 0.1f, 20.0f);
	glTranslatef(0.0f, 0.0f, -_cam->getDistance());

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	//This really has to come from your camera....
	//gluLookAt(10.0f, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f);

	glColor3f(1.0f, 0.0f, 0.0f);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(1.5);
	glBegin(GL_LINES);
		// draw line for x axis
		glColor3f(4.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(4.0, 0.0, 0.0);
		// draw line for y axis
		glColor3f(0.0, 4.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 4.0, 0.0);
		// draw line for Z axis
		glColor3f(0.0, 0.0, 4.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 4.0);
	glEnd();

	//Restore View
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
}

void Renderer::drawCubeWireframe(void)
{
	glPushAttrib(GL_POLYGON_BIT | GL_LIGHTING_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);

	glColor3f(0.2f, 0.2f, 0.2f);
	drawCubeFaces();

	glPopAttrib();
}


void Renderer::enableClipPlanes(void)
{
	for (int i = 0; i<_numClipPlanes; ++i)
		_clipPlanes[i].enable();
}


void Renderer::disableClipPlanes(void)
{
	for (int i = 0; i<_numClipPlanes; ++i)
		_clipPlanes[i].disable();
}


void Renderer::loadGLSLShader(char *defines)
{
	char *vertexShader[] = { "shader/volic_vertex.glsl" };
	char *vectorFieldFragShader[] = { "shader/vectorfield_fragment.glsl" };
	char *bgFragShader[] = { "shader/background_fragment.glsl" };

	char *licRaycastFragShader[] = { "shader/inc_header.glsl",
		"shader/inc_lic.glsl",
		"shader/inc_illum.glsl",
		"shader/lic3d_fragment.glsl",
	};

	char *licSlicingFragShader[] = { "shader/inc_header.glsl",
		"shader/inc_lic.glsl",
		"shader/inc_illum.glsl",
		"shader/lic3d_slicing_fragment.glsl"
	};
	char *licSlicingBlendFragShader[] = { "shader/inc_header.glsl",
		"shader/inc_lic.glsl",
		"shader/inc_illum.glsl",
		"shader/lic3d_slicingblend_fragment.glsl"
	};

	char *licVolumeFragShader[] = { "shader/inc_header.glsl",
		"shader/inc_lic.glsl",
		"shader/lic3d_volume_fragment.glsl",
	};

	char *raycastLICVolumeFragShader[] = { 
		"shader/inc_header.glsl",
		"shader/inc_illum.glsl",
		"shader/raycast_lic3d_fragment.glsl", };

	char *phongVertexShader[] = { "shader/phong_vertex.glsl" };
	char *phongFragmentShader[] = { "shader/phong_fragment.glsl" };

	if (!_volumeShader.loadShader(1, reinterpret_cast<char**>(vertexShader),
		1, reinterpret_cast<char**>(vectorFieldFragShader),
		defines))
	{
		std::cerr << "Renderer:  Error loading Vertex and Fragment Program "
			<< "for Volume Shader." << std::endl;
	}

	_paramVolume.getMemoryLocations(_volumeShader.getProgramObj(), _debug);


	if (!_bgShader.loadShader(1, reinterpret_cast<char**>(vertexShader),
		1, reinterpret_cast<char**>(bgFragShader),
		defines))
	{
		std::cerr << "Renderer:  Error loading Vertex and Fragment Program "
			<< "for Background Shader." << std::endl;
	}
	_paramBackground.getMemoryLocations(_bgShader.getProgramObj(), _debug);


	if (!_raycastShader.loadShader(1, reinterpret_cast<char**>(vertexShader),
		4, reinterpret_cast<char**>(licRaycastFragShader),
		defines))
	{
		std::cerr << "Renderer:  Error loading Vertex and Fragment Program "
			<< "for Volume Shader." << std::endl;
	}
	_paramRaycast.getMemoryLocations(_raycastShader.getProgramObj(), _debug);


	if (!_sliceShader.loadShader(1, reinterpret_cast<char**>(vertexShader),
		4, reinterpret_cast<char**>(licSlicingFragShader),
		defines))
	{
		std::cerr << "Renderer:  Error loading Vertex and Fragment Program "
			<< "for Slicing Shader." << std::endl;
	}
	_paramSlice.getMemoryLocations(_sliceShader.getProgramObj());


	if (!_sliceBlendShader.loadShader(1, reinterpret_cast<char**>(vertexShader),
		4, reinterpret_cast<char**>(licSlicingBlendFragShader),
		defines))
	{
		std::cerr << "Renderer:  Error loading Vertex and Fragment Program "
			<< "for Slicing Shader (blending)." << std::endl;
	}
	_paramSliceBlend.getMemoryLocations(_sliceBlendShader.getProgramObj());


	if (!_phongShader.loadShader(1, reinterpret_cast<char**>(phongVertexShader),
		1, reinterpret_cast<char**>(phongFragmentShader),
		defines))
	{
		std::cerr << "Renderer:  Error loading Vertex and Fragment Program "
			<< "for Phong Shader." << std::endl;
	}
	_paramRaycast.getMemoryLocations(_raycastShader.getProgramObj(), _debug);

	if (!_volumeRenderShader.loadShader(1, reinterpret_cast<char**>(vertexShader),
		3, reinterpret_cast<char**>(licVolumeFragShader),
		defines))
	{
		std::cerr << "Renderer:  Error loading Vertex and Fragment Program "
			<< "for volumeRenderShader Shader." << std::endl;
	}
	_paramLICVolume.getMemoryLocations(_volumeRenderShader.getProgramObj(), _debug);
	if (!_licRaycastShader.loadShader(1, reinterpret_cast<char**>(vertexShader),
		3, reinterpret_cast<char**>(raycastLICVolumeFragShader),
		defines))
	{
		std::cerr << "Renderer:  Error loading Vertex and Fragment Program "
			<< "for raycastLICVolumeFragShader Shader." << std::endl;
	}
	_paramLicRaycast.getMemoryLocations(_licRaycastShader.getProgramObj(), _debug);


	CHECK_FOR_OGL_ERROR();
}


void Renderer::setRenderVolParams(GLSLParamsLIC *param)
{
	if (!_licParams)
		return;

	if (param->viewport > -1)
		glUniform4iARB(param->viewport, 0, 0, _renderWidth, _renderHeight);
	CHECK_FOR_OGL_ERROR();

	if (param->texMax > -1)
		glUniform4fARB(param->texMax,
			_vd->extent[0] * _vd->scale[0],
			_vd->extent[1] * _vd->scale[1],
			_vd->extent[2] * _vd->scale[2], 0.0f);
	CHECK_FOR_OGL_ERROR();

	if (param->scaleVol > -1)
		glUniform4fvARB(param->scaleVol, 1, _vd->scale);
	if (param->scaleVolInv > -1)
		glUniform4fvARB(param->scaleVol, 1, _vd->scaleInv);
	CHECK_FOR_OGL_ERROR();

	if (_lowRes)
	{
		if (param->stepSize > -1)
			glUniform1fARB(param->stepSize, 2.0f*_licParams->stepSizeVol);
		if (param->gradient > -1)
			glUniform3fARB(param->gradient, _licParams->gradientScale,
				_licParams->illumScale, 0.7f*_licParams->freqScale);

		if (param->licParams > -1)
			glUniform3fARB(param->licParams, 15.0f, 15.0f, 1.0f / 64.0f);
		if ((param->licKernel > -1) && _licFilter)
		{
			float invFilterSize = _licFilter->getInverseFilterArea() / (30.0f);
			glUniform3fARB(param->licKernel, 0.5f / 15.0f,
				0.5f / 15.0f, invFilterSize);
		}

		if (param->alphaCorrection > -1)
			glUniform1fARB(param->alphaCorrection, 2.0f*_licParams->stepSizeVol * 128.0f);
	}
	else
	{
		if (param->stepSize > -1)
			glUniform1fARB(param->stepSize, _licParams->stepSizeVol);
		CHECK_FOR_OGL_ERROR();
		if (param->gradient > -1)
			glUniform3fARB(param->gradient, _licParams->gradientScale, _licParams->illumScale, _licParams->freqScale);
		CHECK_FOR_OGL_ERROR();

		if (param->licParams > -1)
			glUniform3fARB(param->licParams, _licParams->stepsForward,
				_licParams->stepsBackward, _licParams->stepSizeLIC);
		CHECK_FOR_OGL_ERROR();
		if ((param->licKernel > -1) && _licFilter)
		{
			float invFilterSize = _licFilter->getInverseFilterArea() / (_licParams->stepsForward + _licParams->stepsBackward);
			glUniform3fARB(param->licKernel, 0.5f / _licParams->stepsForward,
				0.5f / _licParams->stepsBackward, invFilterSize);
		}
		CHECK_FOR_OGL_ERROR();

		if (param->alphaCorrection > -1)
			glUniform1fARB(param->alphaCorrection, _licParams->stepSizeVol * 128.0f);
	}
	CHECK_FOR_OGL_ERROR();

	if (param->numIterations > -1)
		glUniform1iARB(param->numIterations, _licParams->numIterations);
	CHECK_FOR_OGL_ERROR();

	if (param->interpSize > -1)
		glUniform1iARB(param->interpSize, _licParams->interpSize);
	if (param->interpStep > -1)
		glUniform1fARB(param->interpStep, float(_licParams->interpStep)/ _licParams->interpSize);
	CHECK_FOR_OGL_ERROR();
}


void Renderer::setRenderVolTextures(GLSLParamsLIC *param)
{

	// bind texture units to samplers
	if (param->volumeSampler > -1)
	{
		glUniform1iARB(param->volumeSampler, _dataTex->texUnit - GL_TEXTURE0_ARB);
		_dataTex->bind();
	}
	if (param->licVolumeSampler > -1)
	{
		glUniform1iARB(param->licVolumeSampler, _licvolumebuffer->getCurrentLayer()->texUnit - GL_TEXTURE0_ARB);
		_licvolumebuffer->getCurrentLayer()->bind();
	}if (param->licVolumeSamplerOld > -1)
	{
		glUniform1iARB(param->licVolumeSamplerOld, _licvolumebuffer->getOldLayer()->texUnit - GL_TEXTURE0_ARB);
		_licvolumebuffer->getOldLayer()->bind();
	}
	if (param->scalarSampler > -1)
	{
		glUniform1iARB(param->scalarSampler, _scalarTex->texUnit - GL_TEXTURE0_ARB);
		_scalarTex->bind();
	}
	if (param->noiseSampler > -1)
	{
		glUniform1iARB(param->noiseSampler, _noiseTex->texUnit - GL_TEXTURE0_ARB);
		_noiseTex->bind();
	}
	if (param->mcOffsetSampler > -1)
	{
		glUniform1iARB(param->mcOffsetSampler, _mcOffsetTex->texUnit - GL_TEXTURE0_ARB);
		_mcOffsetTex->bind();
	}
	if (param->transferRGBASampler > -1)
	{
		glUniform1iARB(param->transferRGBASampler, _tfRGBTex->texUnit - GL_TEXTURE0_ARB);
		_tfRGBTex->bind();
	}
	if (param->transferAlphaOpacSampler > -1)
	{
		glUniform1iARB(param->transferAlphaOpacSampler, _tfAlphaOpacTex->texUnit - GL_TEXTURE0_ARB);
		_tfAlphaOpacTex->bind();
	}
	if (param->licKernelSampler > -1)
	{
		glUniform1iARB(param->licKernelSampler, _licKernelTex->texUnit - GL_TEXTURE0_ARB);
		_licKernelTex->bind();
	}
	if (param->malloDiffSampler > -1)
	{
		glUniform1iARB(param->malloDiffSampler, _illumMalloDiffTex->texUnit - GL_TEXTURE0_ARB);
		_illumMalloDiffTex->bind();
	}
	if (param->malloSpecSampler > -1)
	{
		glUniform1iARB(param->malloSpecSampler, _illumMalloSpecTex->texUnit - GL_TEXTURE0_ARB);
		_illumMalloSpecTex->bind();
	}
	if (param->zoecklerSampler > -1)
	{
		glUniform1iARB(param->zoecklerSampler, _illumZoecklerTex->texUnit - GL_TEXTURE0_ARB);
		_illumZoecklerTex->bind();
	}
	CHECK_FOR_OGL_ERROR();
}


void Renderer::renderVolume(void)
{
	_volumeShader.enableShader();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glDisable(GL_BLEND);
	glEnable(GL_BLEND);

	setRenderVolParams(&_paramVolume);
	setRenderVolTextures(&_paramVolume);

	drawCubeFaces();
	drawClippedPolygon();

	CHECK_FOR_OGL_ERROR();


	_dataTex->unbind();
	_tfRGBTex->unbind();
	_tfAlphaOpacTex->unbind();

	glDisable(GL_CULL_FACE);
}


void Renderer::raycastVolume(void)
{
	_raycastShader.enableShader();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (!_storeFrame && !_useFBO)
		glEnable(GL_BLEND);

	setRenderVolParams(&_paramRaycast);
	setRenderVolTextures(&_paramRaycast);

	drawCubeFaces();
	drawClippedPolygon();

	CHECK_FOR_OGL_ERROR();


	_dataTex->unbind();
	_tfRGBTex->unbind();
	_tfAlphaOpacTex->unbind();

	glDisable(GL_CULL_FACE);
	if (!_storeFrame && !_useFBO)
		glDisable(GL_BLEND);
	_raycastShader.disableShader();
}


void Renderer::sliceVolume(void)
{
	Texture *tex = NULL;

	if (_paramSlice.imageFBOSampler < 0)
		return;


	_sliceShader.enableShader();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	//glEnable(GL_CULL_FACE);

	if (_useFBO)
	{
		glDisable(GL_BLEND);
		_sliceShader.enableShader();
		setRenderVolParams(&_paramSlice);
		setRenderVolTextures(&_paramSlice);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _framebuffer);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT,
			_imgBufferTex0->texTarget,
			_imgBufferTex0->id, 0);
	}
	else
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
		glEnable(GL_BLEND);

		_sliceBlendShader.enableShader();
		setRenderVolParams(&_paramSliceBlend);
		setRenderVolTextures(&_paramSliceBlend);
	}
	CHECK_FOR_OGL_ERROR();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/*
	if (earlyZ)
	{
	_earlyZShader.enableShader();
	setRenderVolParams(&_paramEarlyZ);
	setRenderVolTextures(&_paramEarlyZ);
	}
	*/

	for (int i = 0; i<_slices.getNumSlices(); ++i)
	{
		/*
		if (earlyZ)
		{
		// update depth mask
		glDepthMask(GL_TRUE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glClear(GL_DEPTH_BUFFER_BIT);

		_earlyZShader.enableShader();

		glBegin(GL_TRIANGLE_FAN);
		{
		_slices.drawSlice(i);
		}
		glEnd();
		CHECK_FOR_OGL_ERROR();

		glDepthMask(GL_FALSE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		_sliceShader.enableShader();
		}
		*/

		if (_useFBO)
		{
			tex = _imgBufferTex0;
			_imgBufferTex0 = _imgBufferTex1;
			_imgBufferTex1 = tex;

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _framebuffer);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				GL_COLOR_ATTACHMENT0_EXT,
				_imgBufferTex0->texTarget,
				_imgBufferTex0->id, 0);
			if (i < 1)
				glClear(GL_COLOR_BUFFER_BIT);
			glUniform1iARB(_paramSlice.imageFBOSampler, _imgBufferTex0->texUnit - GL_TEXTURE0_ARB);
			_imgBufferTex1->bind();
		}

		// draw slice
		glBegin(GL_TRIANGLE_FAN);
		{
			_slices.drawSlice(i);
		}
		glEnd();
		CHECK_FOR_OGL_ERROR();
	}

	_dataTex->unbind();
	_tfRGBTex->unbind();
	_tfAlphaOpacTex->unbind();

	glUseProgramObjectARB(0);

	if (!_useFBO)
	{
		// draw a screen-filling white plane and blend it (for background)
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0.0, 1.0, 0.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glBegin(GL_QUADS);
		{
			glVertex2f(-1.0, -1.0);
			glVertex2f(1.0, -1.0);
			glVertex2f(1.0, 1.0);
			glVertex2f(-1.0, 1.0);
		}
		glEnd();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		CHECK_FOR_OGL_ERROR();

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}

	glDisable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
}


void Renderer::updateSlices(void)
{
	float axis[3];
	GLfloat m[16];

	_cam->getAxis(axis);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glTranslatef(_cam->getPosition().x, _cam->getPosition().y, _cam->getPosition().z);
	glTranslatef(0.0, 0.0, -_cam->getDistance());
	//glRotatef(animatedAngle, 0.0, 1.0, 0.0);
	glRotatef(_cam->getAngle()*180.0f / (float)M_PI, axis[0], axis[1], axis[2]);
	glTranslatef(-_vd->center[0], -_vd->center[1], -_vd->center[2]);

	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	glPopMatrix();

	_slices.setupSlicing(m, (_lowRes ? 2.0f*_licParams->stepSizeVol : _licParams->stepSizeVol), _vd->extent);
}


void Renderer::drawClippedPolygon(void)
{
	// check for each clip plane
	for (int i = 0; i<_numClipPlanes; ++i)
	{
		if (_clipPlanes[i].isActive())
		{
			_slices.setupSingleSlice(_clipPlanes[i].getNormal(), _vd->extent);
			glBegin(GL_TRIANGLE_FAN);
			{
				_slices.drawSingleSlice(-(_clipPlanes[i].getNormal()[3] - 0.0001f));
			}
			glEnd();
		}
	}
}

void Renderer::renderLICVolume(void)
{
	int oldViewport[4];
	float color[4];
	int depth = _licvolumebuffer->getDepth();
	int width = _licvolumebuffer->getWidth();
	int height = _licvolumebuffer->getHeight();
	GLint currentFBO = 0;

	// store current framebuffer object
	glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFBO);

	_licvolumebuffer->bind();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	glGetIntegerv(GL_VIEWPORT, oldViewport);
	glViewport(0, 0, width, height);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	_volumeRenderShader.enableShader();

	setRenderVolParams(&_paramLICVolume);
	setRenderVolTextures(&_paramLICVolume);

	CHECK_FOR_OGL_ERROR();
	
	for (int z = 0; z < depth; z++)
	{
		_licvolumebuffer->attachLayer(0, z);
		//render volume to 3D Texture
		_licvolumebuffer->drawSlice((z + 0.5f) / (float)depth);
	}
	_volumeRenderShader.disableShader();
	
	// restore old clear color
	glClearColor(color[0], color[1], color[2], color[3]);
	
	glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	_licvolumebuffer->unbind();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, currentFBO);
	
	CHECK_FRAMEBUFFER_STATUS();
}

void Renderer::updateLICVolume(void)
{
	_licvolumebuffer->restoreOldLayer();
	renderLICVolume();
}

void Renderer::restoreLICVolume(void)
{
	_licvolumebuffer->restoreOldLayer();
}

void Renderer::raycastLICVolume(void)
{
	_licRaycastShader.enableShader();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glDisable(GL_BLEND);
	glEnable(GL_BLEND);

	setRenderVolParams(&_paramLicRaycast);
	setRenderVolTextures(&_paramLicRaycast);

	drawCubeFaces();
	drawClippedPolygon();

	CHECK_FOR_OGL_ERROR();


	_dataTex->unbind();
	_tfRGBTex->unbind();
	_tfAlphaOpacTex->unbind();

	glDisable(GL_CULL_FACE);
}

void Renderer::renderBackground(void)
{
	int viewport[4] = { 0, 0, _winWidth, _winHeight };
	double x, y, z;
	double modelview[16], projection[16], vertizes[4][3];

	// draw a quad showing the previous image
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	// reset viewport when rendering low resolution FBOs
	if (_lowRes)
		glViewport(0, 0, _winWidth, _winHeight);

	// enable shader
	_bgShader.enableShader();


	if (_screenShot || _recording)
	{
		// render into second fbo texture
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _framebuffer);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_RECTANGLE_ARB,
			_imgBufferTex1->id, 0);
		CHECK_FRAMEBUFFER_STATUS();
		CHECK_FOR_OGL_ERROR();
	}

	// set params
	if (_paramBackground.viewport > -1)
		glUniform4fARB(_paramBackground.viewport,
			static_cast<float>(_renderWidth) / _winWidth,
			static_cast<float>(_renderHeight) / _winHeight,
			0.0f, 0.0f);
	CHECK_FOR_OGL_ERROR();

	if (_paramBackground.imageFBOSampler > -1)
		glUniform1iARB(_paramBackground.imageFBOSampler,
			_imgBufferTex0->texUnit - GL_TEXTURE0_ARB);

	CHECK_FOR_OGL_ERROR();

	_imgBufferTex0->bind();


	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	CHECK_FOR_OGL_ERROR();

	for (int i = 0; i<4; ++i)
	{
		x = ((i & 1) >> 0) * viewport[2];
		y = ((i & 2) >> 1) * viewport[3];
		z = 0.5f;
		gluUnProject(x, y, z, modelview, projection, viewport,
			&vertizes[i][0], &vertizes[i][1], &vertizes[i][2]);
	}

	glBegin(GL_QUADS);
	{
		vertexdv(vertizes[0]);
		vertexdv(vertizes[1]);
		vertexdv(vertizes[3]);
		vertexdv(vertizes[2]);
	}
	glEnd();

	_imgBufferTex0->unbind();
	GLSLShader::disableShader();

	if (_screenShot || _recording)
	{
		// disable fbo texture
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_RECTANGLE_ARB,
			0, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		std::string animationFile = "snapshotOut\\";
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);

		std::stringstream ss;
		ss << animationFile;
		if (_isAnimationOn)
			ss << frames << "_" << _snapshotFileName;
		else 
			ss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S ") << _snapshotFileName;
		auto str = ss.str();

		// TODO: screenshot filename
		saveTexture(str.c_str(), _imgBufferTex0, 4, 15, 255.0);
		std::cout << "Screenshot written to \"" << str
			<< "\"." << std::endl;

		if(_screenShot)
			_screenShot = false;
		if (_recording)
			frames++;
	}

	glDepthMask(GL_TRUE);
	CHECK_FOR_OGL_ERROR();
}

