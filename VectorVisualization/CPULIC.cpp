#include "CPULIC.h"

#include <random>
#include <iostream>
#include <string>
#include <algorithm>

inline int getoffset(int x, int y, int z, int* size)
{
	x = x < size[0] ? x : size[0] - 1;
	y = y < size[1] ? y : size[1] - 1;
	z = z < size[2] ? z : size[2] - 1;
	return x * size[0] * size[1] + y * size[0] + z;
}

CPULIC::CPULIC(int w, int h, int d)
{
	width = w;
	height = h;
	depth = d;

	int size = width * height * depth;

	data = new float[size];
	std::fill_n(data, size, 0);

	iterNum = 1;
	stepSize = 1.0 / width;

	createFilter();
}


CPULIC::~CPULIC()
{
}

Vector3 CPULIC::textureSample(int timestep, Vector3 pos, std::deque<VolumeData*>& vds)
{
	if (vds.size() == 0)
		return Vector3_new(0,0,0);
	VolumeData* vd = vds[timestep];
	int x = pos.x * vd->size[0];
	int y = pos.y * vd->size[1];
	int z = pos.z * vd->size[2];
	int offset = z * vd->size[0] * vd->size[1] + y * vd->size[0] + x;
	//Vector3 dir = ((Vector3*)(vd->data))[offset];
	float* vector4 = (float*)(vtdata + offset * 4 * 4);
	Vector3 res = Vector3_new(vector4[0], vector4[1], vector4[2]);
	return res;
}

inline float clip(float &n, float lower, float upper)
{
	return max(lower, min(n, upper));
}

Vector3 CPULIC::streamlineforward(Vector3 pos, int timestep, std::deque<VolumeData*>& vds)
{
	Vector3 dir = textureSample(timestep, pos, vds);
	dir = Vector3_smult(2, dir);
	dir = Vector3_sub(dir, Vector3_new(1, 1, 1));
	//std::cout << dir.x << " " << dir.y << " " << dir.z << " " << std::endl;
	pos = Vector3_add(pos, Vector3_smult(stepSize, dir));
	clip(pos.x, 0, 1);
	clip(pos.y, 0, 1);
	clip(pos.z, 0, 1);
	return pos;
}

Vector3 CPULIC::streamlinebackward(Vector3 pos, int timestep, std::deque<VolumeData*>& vds)
{
	Vector3 dir = textureSample(timestep, pos, vds);
	dir = Vector3_smult(-2, dir);
	dir = Vector3_add(dir, Vector3_new(1, 1, 1));
	//if (Vector3_normalize(&dir) > 0)
		//std::cout << dir.x << " " << dir.y << " " << dir.z << " " << std::endl;
	pos = Vector3_sub(pos, Vector3_smult(stepSize, dir));
	return pos;
}

float CPULIC::getNoise( int offset)
{
	unsigned char* data_value = (nsdata + offset * 4);
	return data_value[3] / 256.0;
}

float CPULIC::NoiseSample(Vector3 pos)
{
	if (noise == nullptr)
		return 0;
	VolumeData* nvd = noise->getVolumeData();
	float x = pos.x * nvd->size[0];
	float y = pos.y * nvd->size[1];
	float z = pos.z * nvd->size[2];
	int x0 = int(x);
	int y0 = int(y);
	int z0 = int(z);
	float xd = x - x0;
	float yd = y - y0;
	float zd = z - z0;
	int c000 = getoffset(x0, y0, z0, nvd->size);
	int c001 = getoffset(x0, y0, z0+1, nvd->size);
	int c010 = getoffset(x0, y0+1, z0, nvd->size);
	int c011 = getoffset(x0, y0+1, z0+1, nvd->size);
	int c100 = getoffset(x0+1, y0, z0, nvd->size);
	int c101 = getoffset(x0+1, y0, z0+1, nvd->size);
	int c110 = getoffset(x0+1, y0+1, z0, nvd->size);
	int c111 = getoffset(x0+1, y0 + 1, z0 + 1, nvd->size);
	float c00 = getNoise(c000) * (1 - xd) + getNoise(c100) * xd;
	float c01 = getNoise(c001) * (1 - xd) + getNoise(c101) * xd;
	float c10 = getNoise(c010) * (1 - xd) + getNoise(c110) * xd;
	float c11 = getNoise(c011) * (1 - xd) + getNoise(c111) * xd;
	float c0 = c00*(1 - yd) + c10*yd;
	float c1 = c01*(1 - yd) + c11*yd;
	float c = c0 * (1 - zd) + c1*zd;
	////int offset = z0 * nvd->size[0] * nvd->size[1] + y0 * nvd->size[0]  + x0;
	////unsigned char data_value = *((unsigned char*)nvd->data + c000);
	return c;
	//return getNoise(c000);
}

void CPULIC::createFilter()
{
	double stdv = 2.5;
	double mean = 127.5;
	double s = 2 * stdv * stdv;
	double sum = 0;
	for (int i = 0; i < 256; i++)
	{
		filter[i] = (exp(-(i - mean)* (i - mean) / s)) / sqrt(M_PI * s);
		sum += filter[i];
	}
	if (sum > 0.5)
		for (int i = 0; i < 15; i++)
			filter[i] = filter[i] * 0.5 / sum;

}

float CPULIC::kernel(float offset)
{
	int index = (0.5 + 0.5 * offset)*255;
	return _filterData[index]*1.0/255;
	//return filter[index];
}

bool CPULIC::outofSpace(Vector3 pos)
{
	if (pos.x < 0 || pos.y < 0 || pos.z < 0)
		return true;
	else if (pos.x >= 1 || pos.y >= 1 || pos.z >= 1)
		return true;
	else 
		return false;
}

void CPULIC::gather3DLIC(std::deque<VolumeData*>& vds)
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			for (int k = 0; k < depth; k++)
			{
				int index = (i * width + j) * height + k;
				Vector3 pos = Vector3_new(float(i) / width, float(j) / height, float(k) / depth);
				float value = 0.0;
				Vector3 dir = textureSample(0, pos, vds);

				if (dir.x == 0.5 && dir.y == 0.5 && dir.z == 0.5)
				{
					data[index] = 0;
					continue;
				}
				//else
				/*{
					data[index] = dir.x;
					continue;
				}*/
				value += NoiseSample(pos) * kernel(0);
				//forward 
				Vector3 newPos = pos;
				for (int itr = 0; itr < iterNum; itr++)
				{
					newPos = streamlineforward(newPos, itr, vds);
					if (outofSpace(newPos))
						break;
					float offset = 1.0 * itr / iterNum;
					value += NoiseSample(newPos) * kernel(offset);
				}
				//backward 
				newPos = pos;
				for (int itr = 0; itr < iterNum; itr++)
				{
					newPos = streamlinebackward(newPos, itr, vds);
					if (outofSpace(newPos))
						break;
					float offset = -1.0 * itr / iterNum;
					value += NoiseSample(newPos) * kernel(offset);
				}
				//if (value > 0)
				//	std::cout << '*';
				//else
				//	std::cout << '.';
				//if(Vector3_normalize(&dir) > 0)
				//value = NoiseSample(pos);
				//if (value > 0)
				//	std::cout << value << std::endl;
				data[index] = value/3;
			}
			//std::cout << std::endl;
		}
		//std::cout << "------------------------------------" << std::endl;
	}
	return;
}

void CPULIC::scatter3DLIC(std::deque<VolumeData*>& vds)
{
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			for (int k = 0; k < depth; k++)
			{
				int index = (i * width + j) * height + k;
				Vector3 pos = Vector3_new(float(i) / width, float(j) / height, float(k) / depth);
				float value = 0.0;
				Vector3 dir = textureSample(0, pos, vds);

				if (dir.x == 0.5 && dir.y == 0.5 && dir.z == 0.5)
				{
					continue;
				}
				//else
				/*{
				data[index] = dir.x;
				continue;
				}*/
				float noise = NoiseSample(pos);
				if (noise <= 0)
					continue;
				data[index] += noise * kernel(0);
				//forward 
				Vector3 newPos = pos;
				for (int itr = 0; itr < iterNum; itr++)
				{
					newPos = streamlineforward(newPos, itr, vds);
					if (outofSpace(newPos))
						break;
					int x = newPos.x * width;
					int y = newPos.y * height;
					int z = newPos.z * depth;
					index = (x * width + y) * height + z;
					float offset = 1.0 * (itr + 1) / iterNum;
					data[index] += noise * kernel(offset);
				}
				//backward 
				newPos = pos;
				for (int itr = 0; itr < iterNum; itr++)
				{
					newPos = streamlinebackward(newPos, itr, vds);
					if (outofSpace(newPos))
						break;
					int x = newPos.x * width;
					int y = newPos.y * height;
					int z = newPos.z * depth;
					index = (x * width + y) * height + z;
					float offset = -1.0 * (itr + 1) / iterNum;
					data[index] += noise * kernel(offset);
				}
				//if (value > 0)
				//	std::cout << '*';
				//else
				//	std::cout << '.';
				//if(Vector3_normalize(&dir) > 0)
				//value = NoiseSample(pos);
				//if (value > 0)
				//	std::cout << value << std::endl;
				//data[index] = value / 3;
			}
			//std::cout << std::endl;
		}
		//std::cout << "------------------------------------" << std::endl;
	}
	return;
}

Texture* CPULIC::createTexture(const char *texName, GLuint texUnit, bool floatTex)
{
	GLuint texId;
	void *paddedData = NULL;

	Texture * tex = new Texture;
	// create a texture id
	if (tex->id == 0)
	{
		glGenTextures(1, &texId);
		tex->setTex(GL_TEXTURE_3D, texId, texName);
	}
	
	tex->texUnit = texUnit;
	tex->width = width;
	tex->height = height;
	tex->depth = depth;

#if FORCE_POWER_OF_TWO_TEXTURE == 1
	if ((_vd->texSize[0] != _vd->size[0])
		|| (_vd->texSize[1] != _vd->size[1])
		|| (_vd->texSize[2] != _vd->size[2]))
	{
		fprintf(stderr, "VectorData:  Dimensions are not 2^n.\n");
	}
#endif
	if (floatTex)
	{
		_texSrcFmt = GL_FLOAT;
		_texIntFmt = GL_LUMINANCE16F_ARB;

	}
	else
	{
		_texSrcFmt = GL_UNSIGNED_BYTE;
		_texIntFmt = GL_LUMINANCE;

	}

	tex->format = _texIntFmt;

	glBindTexture(GL_TEXTURE_3D, tex->id);
	CHECK_FOR_OGL_ERROR();

	glTexImage3D(GL_TEXTURE_3D, 0, _texIntFmt, width, height, depth, 
		0, GL_LUMINANCE, _texSrcFmt, data);
	CHECK_FOR_OGL_ERROR();
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	CHECK_FOR_OGL_ERROR();

	return tex;
}

void CPULIC::generateTexture()
{
	tex = createTexture("CPULIC_Tex", GL_TEXTURE2_ARB);
}

void CPULIC::getNoiseTextureData()
{
	GLuint noiseTexId = noise->getTextureRef()->id;
	glBindTexture(GL_TEXTURE_3D, noiseTexId);
	GLint width, height, depth, internalFormat;
	glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_COMPONENTS, &internalFormat); // get internal format type of GL texture
	glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width); // get width of GL texture
	glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height); // get height of GL texture
	glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth); // get depth of GL texture

	GLint numBytes = 0;
	switch (internalFormat) // determine what type GL texture has...
	{
	case GL_RGB:
		numBytes = width * height * depth * 3;
		break;
	case GL_RGBA:
		numBytes = width * height *depth * 4;
		break;
	default: // unsupported type (or you can put some code to support more formats if you need)
		break;
	}

	if (numBytes)
	{
		nsdata = (unsigned char*)malloc(numBytes); // allocate image data into RAM
		glGetTexImage(GL_TEXTURE_3D, 0, internalFormat, GL_UNSIGNED_BYTE, nsdata);
	}
	glBindTexture(GL_TEXTURE_3D, 0);
}

void CPULIC::getVectorTextureData()
{
	GLuint vectorTexId = tex->id;
	glBindTexture(GL_TEXTURE_3D, vectorTexId);
	GLint width, height, depth, internalFormat;
	glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_COMPONENTS, &internalFormat); // get internal format type of GL texture
	glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width); // get width of GL texture
	glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height); // get height of GL texture
	glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth); // get depth of GL texture

	GLint numBytes = 0;
	switch (internalFormat) // determine what type GL texture has...
	{
	case GL_RGBA:
		numBytes = width * height * depth * 4;
		break;
	case GL_RGBA16F_ARB:
		numBytes = width * height *depth * 4 * 4;
		break;
	default: // unsupported type (or you can put some code to support more formats if you need)
		break;
	}
	if (numBytes)
	{
		vtdata = (unsigned char*)malloc(numBytes); // allocate image data into RAM
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, vtdata);
	}
	CHECK_FOR_OGL_ERROR();
	glBindTexture(GL_TEXTURE_3D, 0);
}
