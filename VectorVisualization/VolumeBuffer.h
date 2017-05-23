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
	void restoreOldLayer();

	Texture* getCurrentLayer() { return &(_tex[0]); }
	Texture* getOldLayer() { return &(_tex[1]); }
	int getDepth() { return _depth; }
	int getWidth() { return _width; }
	int getHeight() { return _height; }

	void setInterpolation(int size) {
		_interpSize = size;
	}
	void increaseInterpoStep() { _curIntepStep++; }
	void setIncreaseInterpoStep(int step) { _curIntepStep = step; }

	bool isAnimation() { return animationFlag; }
	void animationOn() { animationFlag = true; }
private:
	int _width, _height, _depth;
	int _maxlayers;
	int _layer;

	bool animationFlag;
	int _interpSize;
	int _curIntepStep;

	GLuint _frambufferId;
	// tex[0] : new tex. tex[1] : old tex.
	Texture _tex[2];
};

