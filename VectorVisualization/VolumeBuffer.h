#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "texture.h"

class VolumeBuffer
{
public:
	VolumeBuffer(GLint format, int width, int height, int depth, int layers);
	~VolumeBuffer();

	GLuint create3dTexture(GLint internalformat, int w, int h, int d);
	void bind();
	void unbind();
	void attachLayer(int layer, int zSlice);
	void attachTexture(GLenum texTarget, GLenum attachment, GLuint texId, int mipLevel, int zSlice);
	void drawSlice(float z);

	Texture* getCurrentLayer() { return &(_tex[_layer]); }
	int getDepth() { return _depth; }
	int getWidth() { return _width; }
	int getHeight() { return _height; }
private:
	int _width, _height, _depth;
	int _maxlayers;
	int _layer;

	GLuint _frambufferId;
	Texture * _tex;
};

