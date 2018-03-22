#include "VolumeBuffer.h"
#include "types.h"

VolumeBuffer::VolumeBuffer(GLint format, int width, int height, int depth, int layers, bool depthFlag)
	:_width(width), _height(height), _depth(depth), _maxlayers(layers), _layer(0),
	_interpSize(0), _curIntepStep(0), animationFlag(false), _depthFlag(depthFlag)
{
	glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
	_frambufferId = 0;
	glGenFramebuffersEXT(1, &_frambufferId);
	//_tex = new Texture[_maxlayers];
	for (int i = 0; i < _maxlayers; i++) {
		_tex[i].setTex(GL_TEXTURE_3D, create3dTexture(format, _width, _height, _depth, GL_RGBA, GL_FLOAT), "LIC_Tex");
		_tex[i].width = _width;
		_tex[i].height = _height;
		_tex[i].depth = _depth;
	}
	_tex[0].texUnit = GL_TEXTURE13_ARB;
	_tex[1].texUnit = GL_TEXTURE14_ARB;

	if (_depthFlag)
	{
		for (int i = 0; i < _maxlayers; i++) {
			_depthtex[i].setTex(GL_TEXTURE_2D_ARRAY, create2darrayTexture(GL_DEPTH_COMPONENT32F, _width, _height, _depth, GL_DEPTH_COMPONENT, GL_FLOAT), "LIC_DepTex");
			_depthtex[i].width = _width;
			_depthtex[i].height = _height;
			_depthtex[i].depth = _depth;
		}
		_depthtex[0].texUnit = GL_TEXTURE15_ARB;
		_depthtex[1].texUnit = GL_TEXTURE16_ARB;

	}
}


VolumeBuffer::~VolumeBuffer()
{
	if (_frambufferId && glFramebufferTexture3DEXT && glBindFramebufferEXT
		&& glDeleteFramebuffersEXT && glDeleteRenderbuffersEXT)
	{
		glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT,
			GL_TEXTURE_RECTANGLE_ARB,
			0, 0, 0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		glDeleteFramebuffersEXT(1, &_frambufferId);
	}
	//if(_frambufferId)
	//glDeleteFramebuffersEXT(1, &_frambufferId);
	for (int i = 0; i<_maxlayers; i++) {
		glDeleteTextures(1, &_tex[i].id);
	}
	if (_depthFlag)
	{
		for (int i = 0; i<_maxlayers; i++) {
			glDeleteTextures(1, &_depthtex[i].id);
		}
		delete[] _depthtex;
	}
	delete [] _tex;
}

GLuint VolumeBuffer::create3dTexture(GLint internalformat, int w, int h, int d, GLenum format, GLenum type)
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
	glTexImage3D(GL_TEXTURE_3D, 0, internalformat, w, h, d, 0, format, type, 0);
	CHECK_FOR_OGL_ERROR();
	return tex;
}

GLuint VolumeBuffer::create2darrayTexture(GLint internalformat, int w, int h, int d, GLenum format, GLenum type)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	GLint mode = GL_REPEAT;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, mode);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalformat, w, h, d, 0, format, type, 0);
	CHECK_FOR_OGL_ERROR();
	return tex;
}

void VolumeBuffer::restoreOldLayer()
{
	if (_tex[0].id > 0 && _tex[1].id > 0)
	{
		glCopyImageSubData(_tex[0].id, GL_TEXTURE_3D, 0, 0, 0, 0, 
			_tex[1].id, GL_TEXTURE_3D, 0, 0, 0, 0, _tex[0].width, _tex[0].height, _tex[0].depth);
		if(_depthFlag && _depthtex[0].id > 0 && _depthtex[1].id > 0)
			glCopyImageSubData(_depthtex[0].id, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0,
				_depthtex[1].id, GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, _depthtex[0].width, _depthtex[0].height, _depthtex[0].depth);

		/*glBindFramebuffer(GL_FRAMEBUFFER, _frambufferId);
		for (int i = 0; i < _tex[0].depth; i++)
		{
			glFramebufferTexture3D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_3D, _tex[0].id, 0, i);
			glFramebufferTexture3D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
				GL_TEXTURE_3D, _tex[1].id, 0, i);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, _tex[0].width, _tex[0].height, 0, 0, _tex[0].width, _tex[0].height,
				GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);*/
		CHECK_FOR_OGL_ERROR();
	}
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
	attachTexture(GL_TEXTURE_3D, GL_COLOR_ATTACHMENT0_EXT + layer, _tex[layer].id, 0, zSlice);

	if(_depthFlag)
		attachTexture(GL_TEXTURE_2D_ARRAY, GL_DEPTH_ATTACHMENT_EXT, _depthtex[layer].id, 0, zSlice);
	//CHECK_FRAMEBUFFER_STATUS();
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
	else if (texTarget == GL_TEXTURE_2D_ARRAY) {
		glFramebufferTextureLayerEXT(GL_FRAMEBUFFER_EXT, attachment,
			texId, mipLevel, zSlice);
	}
	else {
		// Default is GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE_ARB, or cube faces
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment,
			texTarget, texId, mipLevel);
	}
	CHECK_FOR_OGL_ERROR();
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