#pragma once
#include "dataset.h"
#include "mmath.h"
#include "texture.h"

class CPULIC
{
public:
	CPULIC(int w, int h, int d);
	~CPULIC();

	void setNoiseData(NoiseDataSet* ns) { noise = ns; }
	void setVectorTexture(Texture* t) { tex = t; }
	void getNoiseTextureData();
	void getVectorTextureData();
	void gather3DLIC(std::deque<VolumeData*>& vds);
	void scatter3DLIC(std::deque<VolumeData*>& vds);
	void generateTexture();
	Texture* getTextureRef() { return tex; }
	void setkernelData(unsigned char * fd) { _filterData = fd; }
private:
	bool outofSpace(Vector3 pos);
	
	Vector3 textureSample(int timestep, Vector3 pos, std::deque<VolumeData*>& vds);
	Vector3 streamlineforward(Vector3 pos, int timestep, std::deque<VolumeData*>& vds);
	Vector3 streamlinebackward(Vector3 pos, int timestep, std::deque<VolumeData*>& vds);
	float NoiseSample(Vector3 pos);
	float kernel(float offset);
	void createFilter();
	Texture* createTexture(const char *texName, GLuint texUnit, bool floatTex = true);
	float getNoise(int offset);

	float* data; // 3D array for texture data

	int width;
	int height;
	int depth;

	int iterNum;
	float stepSize;
	float filter[256];
	unsigned char *_filterData;

	GLint  _texIntFmt;
	GLenum _texSrcFmt;

	Texture* tex;
	NoiseDataSet* noise;
	unsigned char* nsdata;
	unsigned char* vtdata;
};

