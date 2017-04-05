#include "VolumeBuffer.h"



VolumeBuffer::VolumeBuffer(GLint format, int width, int height, int depth, int layers)
	:_width(width), _height(height), _depth(depth), _maxlayers(layers), _layer(0)
{
	glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
	_frambufferId = 0;
	glGenFramebuffersEXT(1, &_frambufferId);
	_tex = new Texture[_maxlayers];
	for (int i = 0; i<_maxlayers; i++) {
		_tex[i].setTex(GL_TEXTURE_3D, create3dTexture(format, _width, _height, _depth), "LIC_Tex");
		_tex[i].width = _width;
		_tex[i].height = _height;
		_tex[i].depth = _depth;
	}
}


VolumeBuffer::~VolumeBuffer()
{
	glDeleteFramebuffersEXT(1, &_frambufferId);
	for (int i = 0; i<_maxlayers; i++) {
		glDeleteTextures(1, &_tex[i].id);
	}
	delete [] _tex;
}

GLuint VolumeBuffer::create3dTexture(GLint internalformat, int w, int h, int d)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLint mode = GL_REPEAT;
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, mode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, mode);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, mode);
	glTexImage3D(GL_TEXTURE_3D, 0, internalformat, w, h, d, 0, GL_RGBA, GL_FLOAT, 0);
	return tex;
}

void VolumeBuffer::bind()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _frambufferId);
}

void VolumeBuffer::unbind()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void VolumeBuffer::attachLayer(int layer, int zSlice)
{
	attachTexture(GL_TEXTURE_3D, GL_COLOR_ATTACHMENT0_EXT, _tex[layer].id, 0, zSlice);
}

void VolumeBuffer::attachTexture(GLenum texTarget, GLenum attachment, GLuint texId, int mipLevel, int zSlice)
{
	if (texTarget == GL_TEXTURE_1D) {
		glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, attachment,
			GL_TEXTURE_1D, texId, mipLevel);
	}
	else if (texTarget == GL_TEXTURE_3D) {
		glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachment,
			GL_TEXTURE_3D, texId, mipLevel, zSlice);
	}
	else {
		// Default is GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB, or cube faces
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment,
			texTarget, texId, mipLevel);
	}
}

void VolumeBuffer::drawSlice(float z)
{
	glBegin(GL_QUADS);
	glTexCoord3f(0.0f, 0.0f, z); glVertex2f(-1.0f, -1.0f);
	glTexCoord3f(1.0f, 0.0f, z); glVertex2f(1.0f, -1.0f);
	glTexCoord3f(1.0f, 1.0f, z); glVertex2f(1.0f, 1.0f);
	glTexCoord3f(0.0f, 1.0f, z); glVertex2f(-1.0f, 1.0f);
	glEnd();
}