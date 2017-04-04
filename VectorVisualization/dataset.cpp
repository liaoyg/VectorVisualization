#ifdef _WIN32
#  include <windows.h>
#endif

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
//#include <GL/glu.h>

#include <iostream>
#include <string>
#include <float.h>
#include <assert.h>

#include "texture.h"
#include "imageUtils.h"
#include "gradient.h"
#include "mmath.h"
#include "reader.h"
#include "types.h"
#include "dataSet.h"


VolumeData::~VolumeData(void)
{
	std::cerr << "Volume data is to be deleted." << std::endl;
	
	for (auto d : dataSets) {
		switch (dataType)
		{
		case DATRAW_UCHAR:
			delete[] static_cast<unsigned char*>(d);
			break;
		case DATRAW_USHORT:
			delete[] static_cast<unsigned short*>(d);
			break;
		case DATRAW_FLOAT:
			delete[] static_cast<float*>(d);
			break;
		case DATRAW_NONE:
		default:
			if (d)
				std::cerr << "~Volume: unknown data type" << std::endl;
			break;
		}
		d = NULL;
	}
	data = NULL;
}


DataSet::DataSet(void) : _fileName(NULL), _loaded(false),
_texIntFmt(GL_RGBA),
_texSrcFmt(GL_UNSIGNED_BYTE)
{
}


DataSet::~DataSet(void)
{
	delete[] _fileName;

	if (_tex.id)
		glDeleteTextures(1, &_tex.id);
}


void DataSet::setFileName(const char *fileName)
{
	delete[] _fileName;
	_fileName = NULL;

	if (!fileName)
		return;

	_fileName = new char[strlen(fileName) + 1];
	strcpy(_fileName, fileName);
}


// --------------------------------------------------

VectorDataSet::VectorDataSet(void)
{
	_vd = new VolumeData();
	interpIndex = 0;
	InterpSize = 1;
}


VectorDataSet::~VectorDataSet(void)
{
	delete _vd;
}


bool VectorDataSet::loadData(const char *fileName)
{
	float volSize[3];
	float maxVolSize = 0;
	int maxTexSize = 0;

	if (fileName)
		setFileName(fileName);
	else if (!_fileName)
	{
		fprintf(stderr, "VectorData:  No filename set.\n");
		return false;
	}

	delete[](unsigned char*)_vd->data;
	_vd->data = NULL;
	_loaded = false;

	if (!_datFile.parseDatFile(_fileName))
	{
		fprintf(stderr, "VectorData:  Could not parse DAT file \"%s\".\n",
			_fileName);
		return false;
	}

	// ensure that raw data consists only of scalar data
	if (_datFile.getDataDimension() != 3)
	{
		fprintf(stderr, "VectorData:  DAT file \"%s\" refers not to a vector data set.\n",
			_fileName);
		return false;
	}
	_vd->dataDim = 3; // 3D vector

					  // copy volume dimensions from DatFile to VolumeData
	_vd->dataType = _datFile.getDataType();

	if ((_vd->dataType != DATRAW_UCHAR)
		&& (_vd->dataType != DATRAW_FLOAT))
	{
		fprintf(stderr, "VectorData:  Only 8bit integer and 32bit "
			"float vectors are supported.\n");
		return false;
	}

	// determine 3D texture dimensions
	// and update slice distances
	for (int i = 0; i<3; ++i)
	{
		// data set dimensions
		_vd->size[i] = _datFile.getDataSizes()[i];
		// texture size
#if FORCE_POWER_OF_TWO_TEXTURE == 1
		_vd->texSize[i] = getNextPowerOfTwo(_vd->size[i]);
#else
		_vd->texSize[i] = _vd->size[i];
#endif
		// distance between slices
		_vd->sliceDist[i] = _datFile.getDataDists()[i];

		volSize[i] = _vd->size[i] * _vd->sliceDist[i];
		if (volSize[i] > maxVolSize)
			maxVolSize = volSize[i];
		if (_vd->texSize[i] > maxTexSize)
			maxTexSize = _vd->texSize[i];
	}
	// set up texture scaling and bounding box extents of the volume
	for (int i = 0; i < 3; ++i)
	{
		_vd->scale[i] = maxTexSize / (_vd->texSize[i] * _vd->sliceDist[i]);
		_vd->scaleInv[i] = _vd->texSize[i] * _vd->sliceDist[i] / maxTexSize;

		_vd->extent[i] = _vd->size[i] * _vd->sliceDist[i] / maxVolSize;
	}
	_vd->scale[3] = 0.0f;
	_vd->scaleInv[3] = 1.0f;

	_vd->center[0] = _vd->extent[0] / 2.0f;
	_vd->center[1] = _vd->extent[1] / 2.0f;
	_vd->center[2] = _vd->extent[2] / 2.0f;

	_loaded = true;
	return true;
}

int VectorDataSet::getNextTimeStep()
{
	return _datFile.getNextTimeStep();
}

int VectorDataSet::NextTimeStep()
{
	return _datFile.NextTimeStep();
}

int VectorDataSet::getCurTimeStep()
{
	return _datFile.getCurTimeStep();
}

void* VectorDataSet::loadTimeStep(int timeStep)
{
	return _datFile.readRawData(timeStep);
}

void VectorDataSet::checkInterpolateStage()
{
	if (interpIndex >= InterpSize)
	{
		_vd->data = _vd->dataSets[getNextTimeStep()];
		_vd->newData = _vd->dataSets[NextTimeStep()];
		interpIndex = 0;
	}
}


void VectorDataSet::createTexture(const char *texName,
	GLuint texUnit,
	bool floatTex)
{
	GLuint texId;
	void *paddedData = NULL;

	if (!_loaded)
		return;

	// create a texture id
	if (_tex.id == 0)
	{
		glGenTextures(1, &texId);
		_tex.setTex(GL_TEXTURE_3D, texId, texName);
	}
	else
	{
		glDeleteTextures(1, &(_tex.id));
		glGenTextures(1, &texId);
		_tex.setTex(GL_TEXTURE_3D, texId, texName);
	}
	_tex.texUnit = texUnit;
	_tex.width = _vd->texSize[0];
	_tex.height = _vd->texSize[1];
	_tex.depth = _vd->texSize[2];


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
		_texIntFmt = GL_RGBA16F_ARB;

		paddedData = fillTexDataFloat();
	}
	else
	{
		_texSrcFmt = GL_UNSIGNED_BYTE;
		_texIntFmt = GL_RGBA;

		paddedData = fillTexDataChar();
	}
	_tex.format = _texIntFmt;

	glBindTexture(GL_TEXTURE_3D, _tex.id);
	glTexImage3D(GL_TEXTURE_3D, 0, _texIntFmt, _vd->texSize[0],
		_vd->texSize[1], _vd->texSize[2], 0, GL_RGBA,
		_texSrcFmt, paddedData);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	CHECK_FOR_OGL_ERROR();

	if (floatTex)
		delete[](float*)paddedData;
	else
		delete[](unsigned char*)paddedData;
	paddedData = NULL;
}

void VectorDataSet::createTextureIterp(const char *texName,
	GLuint texUnit,
	bool floatTex)
{
	GLuint texId;
	void *paddedData = NULL;

	if (!_loaded)
		return;

	// create a texture id
	if (_tex.id == 0)
	{
		glGenTextures(1, &texId);
		_tex.setTex(GL_TEXTURE_3D, texId, texName);
	}
	else
	{
		glDeleteTextures(1, &(_tex.id));
		glGenTextures(1, &texId);
		_tex.setTex(GL_TEXTURE_3D, texId, texName);
	}
	_tex.texUnit = texUnit;
	_tex.width = _vd->texSize[0];
	_tex.height = _vd->texSize[1];
	_tex.depth = _vd->texSize[2];


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
		_texIntFmt = GL_RGBA16F_ARB;

		paddedData = fillTexDataFloatInterp();
	}
	else
	{
		_texSrcFmt = GL_UNSIGNED_BYTE;
		_texIntFmt = GL_RGBA;

		paddedData = fillTexDataCharInterp();
	}

	_tex.format = _texIntFmt;

	glBindTexture(GL_TEXTURE_3D, _tex.id);
	glTexImage3D(GL_TEXTURE_3D, 0, _texIntFmt, _vd->texSize[0],
		_vd->texSize[1], _vd->texSize[2], 0, GL_RGBA,
		_texSrcFmt, paddedData);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	CHECK_FOR_OGL_ERROR();

	if (floatTex)
		delete[](float*)paddedData;
	else
		delete[](unsigned char*)paddedData;
	paddedData = NULL;
}

void VectorDataSet::createTextures(const char *texName, int datasize,
	GLuint texUnit,
	bool floatTex)
{
	for (int i = 0; i < datasize; i++)
	{
		GLuint texId;
		void *paddedData = NULL;

		if (!_loaded)
			return;

		Texture tex;
		char texSetName[100];
		strcpy(texSetName, texName);
		strcat(texSetName, std::to_string(i).c_str());

		// create a texture id
		if (tex.id == 0)
		{
			glGenTextures(1, &texId);
			tex.setTex(GL_TEXTURE_3D, texId, texSetName);
		}
		tex.texUnit = texUnit;
		tex.width = _vd->texSize[0];
		tex.height = _vd->texSize[1];
		tex.depth = _vd->texSize[2];

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
			_texIntFmt = GL_RGBA16F_ARB;

			paddedData = fillTexDataFloat();
		}
		else
		{
			_texSrcFmt = GL_UNSIGNED_BYTE;
			_texIntFmt = GL_RGBA;

			paddedData = fillTexDataChar();
		}

		tex.format = _texIntFmt;

		glBindTexture(GL_TEXTURE_3D, tex.id);
		glTexImage3D(GL_TEXTURE_3D, 0, _texIntFmt, _vd->texSize[0],
			_vd->texSize[1], _vd->texSize[2], 0, GL_RGBA,
			_texSrcFmt, paddedData);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		CHECK_FOR_OGL_ERROR();

		if (floatTex)
			delete[](float*)paddedData;
		else
			delete[](unsigned char*)paddedData;
		paddedData = NULL;

		_texSet.push_back(tex);
	}
}

void* VectorDataSet::fillTexDataFloat(void)
{
	int size = _vd->texSize[0] * _vd->texSize[1] * _vd->texSize[2];
	int adr;
	int adrPacked;
	float len;
	float maxLen = -1.0;

	unsigned char *dataU = (unsigned char*)_vd->data;
	float *dataF = (float*)_vd->data;
	float *padded = new float[4 * size];

	memset(padded, 0, 4 * size * sizeof(float));

	// determine maximum length in the vector field
	// and normalize the direction
	// the original magnitude is stored in the forth channel
	for (int z = 0; z<_vd->size[2]; ++z)
	{
		for (int y = 0; y<_vd->size[1]; ++y)
		{
			for (int x = 0; x<_vd->size[0]; ++x)
			{
				adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
				adr = (z*_vd->size[1] + y)*_vd->size[0] + x;

				if (_vd->dataType == DATRAW_UCHAR)
				{
					len = sqrt((float)SQR(dataU[3 * adr] - 128) + (float)SQR(dataU[3 * adr + 1] - 128) + (float)SQR(dataU[3 * adr + 2] - 128));
					if (len < EPS)
					{
						len = 0.0f;
						padded[4 * adrPacked] = 0.005f;
						padded[4 * adrPacked + 1] = 0.005f;
						padded[4 * adrPacked + 2] = 0.005f;
					}
					else
					{
						padded[4 * adrPacked] = 0.5f*(dataU[3 * adr] - 128) / len + 0.5f;
						padded[4 * adrPacked + 1] = 0.5f*(dataU[3 * adr + 1] - 128) / len + 0.5f;
						padded[4 * adrPacked + 2] = 0.5f*(dataU[3 * adr + 2] - 128) / len + 0.5f;
					}
				}
				else
				{
					len = sqrt(SQR(dataF[3 * adr]) + SQR(dataF[3 * adr + 1]) + SQR(dataF[3 * adr + 2]));
					if (len < EPS)
					{
						len = 0.0f;
						padded[4 * adrPacked] = 0.5f;
						padded[4 * adrPacked + 1] = 0.5f;
						padded[4 * adrPacked + 2] = 0.5f;
					}
					else
					{
						padded[4 * adrPacked] = 0.5f*dataF[3 * adr] / len + 0.5f;
						padded[4 * adrPacked + 1] = 0.5f*dataF[3 * adr + 1] / len + 0.5f;
						padded[4 * adrPacked + 2] = 0.5f*dataF[3 * adr + 2] / len + 0.5f;
					}
				}

				if (len > maxLen)
					maxLen = len;

				// store magnitude as float for higher precision
				padded[4 * adrPacked + 3] = len;
			}
		}
	}

	// adapt the range of the magnitude to [0,1]
	//for (int z = 0; z<_vd->size[2]; ++z)
	//	for (int y = 0; y<_vd->size[1]; ++y)
	//		for (int x = 0; x<_vd->size[0]; ++x)
	//		{
	//			adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
	//			len = padded[4 * adrPacked + 3] / maxLen;
	//			padded[4 * adrPacked + 3] = (len > 1.0f) ? 1.0f : ((len < 0.0f) ? 0.0f : len);
	//		}

	return padded;
}

// make tecture data between two key frame
// this is for creating texture data for animation
void* VectorDataSet::fillTexDataFloatInterp()
{
	int size = _vd->texSize[0] * _vd->texSize[1] * _vd->texSize[2];
	int adr;
	int adrPacked;
	float len;
	float maxLen = -1.0;

	unsigned char *dataU = (unsigned char*)_vd->data;
	unsigned char *dataNextU = (unsigned char*)_vd->newData;
	float *dataF = (float*)_vd->data;
	float *dataNextF = (float*)_vd->newData;
	float *padded = new float[4 * size];

	memset(padded, 0, 4 * size * sizeof(float));

	// determine maximum length in the vector field
	// and normalize the direction
	// the original magnitude is stored in the forth channel
	for (int z = 0; z<_vd->size[2]; ++z)
	{
		for (int y = 0; y<_vd->size[1]; ++y)
		{
			for (int x = 0; x<_vd->size[0]; ++x)
			{
				adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
				adr = (z*_vd->size[1] + y)*_vd->size[0] + x;

				unsigned char tdataU[3];
				float tdataF[3];

				if (_vd->dataType == DATRAW_UCHAR)
				{
					for (int idx = 0; idx < 3; idx++)
					{
						dataU[3 * adr + idx] -= 128;
						dataNextU[3 * adr + idx] -= 128;
						tdataU[idx] = dataU[3 * adr + idx] + (float)interpIndex / InterpSize * (dataNextU[3 * adr + idx] - dataU[3 * adr + idx]);
					}
					len = sqrt((float)SQR(tdataU[0]) + (float)SQR(tdataU[1]) + (float)SQR(tdataU[2]));
					if (len < EPS)
					{
						len = 0.0f;
						padded[4 * adrPacked] = 0.5f;
						padded[4 * adrPacked + 1] = 0.5f;
						padded[4 * adrPacked + 2] = 0.5f;
					}
					else
					{
						padded[4 * adrPacked] = 0.5f*(tdataU[0]) / len + 0.5f;
						padded[4 * adrPacked + 1] = 0.5f*(tdataU[1]) / len + 0.5f;
						padded[4 * adrPacked + 2] = 0.5f*(tdataU[2]) / len + 0.5f;
					}
				}
				else
				{/*
					if (dataF[3 * adr] > 0 && interpIndex == 48)
						std::cout << dataF[3 * adr] << " " << dataNextF[3 * adr] << std::endl;*/
					for (int idx = 0; idx < 3; idx++)
					{
						tdataF[idx] = dataF[3 * adr + idx] + (float)interpIndex / InterpSize * (dataNextF[3 * adr + idx] - dataF[3 * adr + idx]);
					}
					len = sqrt(SQR(tdataF[0]) + SQR(tdataF[1]) + SQR(tdataF[2]));
					if (len < EPS)
					{
						len = 0.0f;
						padded[4 * adrPacked] = 0.5f;
						padded[4 * adrPacked + 1] = 0.5f;
						padded[4 * adrPacked + 2] = 0.5f;
					}
					else
					{
						padded[4 * adrPacked] = 0.5f*tdataF[0] / len + 0.5f;
						padded[4 * adrPacked + 1] = 0.5f*tdataF[1] / len + 0.5f;
						padded[4 * adrPacked + 2] = 0.5f*tdataF[2] / len + 0.5f;
						/*padded[4 * adrPacked] = tdataF[0] / len;
						padded[4 * adrPacked + 1] = tdataF[1] / len;
						padded[4 * adrPacked + 2] = tdataF[2] / len;*/
					}
				}

				if (len > maxLen)
					maxLen = len;

				// store magnitude as float for higher precision
				padded[4 * adrPacked + 3] = len;
			}
		}
	}

	// adapt the range of the magnitude to [0,1]
	for (int z = 0; z<_vd->size[2]; ++z)
		for (int y = 0; y<_vd->size[1]; ++y)
			for (int x = 0; x<_vd->size[0]; ++x)
			{
				adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
				len = padded[4 * adrPacked + 3] / maxLen;
				padded[4 * adrPacked + 3] = (len > 1.0f) ? 1.0f : ((len < 0.0f) ? 0.0f : len);
			}

	interpIndex++;
	return padded;
}

void* VectorDataSet::fillTexDataChar(void)
{
	int size = _vd->texSize[0] * _vd->texSize[1] * _vd->texSize[2];
	int adr;
	int adrPacked;
	float len;
	float maxLen = -1.0;

	float *magnitude = new float[size];

	unsigned char *dataU = (unsigned char*)_vd->data;
	float *dataF = (float*)_vd->data;
	unsigned char *padded = new unsigned char[4 * size];

	memset(padded, 0, 4 * size * sizeof(char));

	// determine maximum length in the vector field
	// and normalize the direction
	// the original magnitude is stored in the forth channel/in *magnitude
	for (int z = 0; z<_vd->size[2]; ++z)
	{
		for (int y = 0; y<_vd->size[1]; ++y)
		{
			for (int x = 0; x<_vd->size[0]; ++x)
			{
				adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
				adr = (z*_vd->size[1] + y)*_vd->size[0] + x;

				if (_vd->dataType == DATRAW_UCHAR)
				{
					len = sqrt((float)SQR(dataU[3 * adr] - 128) + (float)SQR(dataU[3 * adr + 1] - 128) + (float)SQR(dataU[3 * adr + 2] - 128));
					if (len < EPS)
					{
						len = 0.0f;
						padded[4 * adrPacked] = 128;
						padded[4 * adrPacked + 1] = 128;
						padded[4 * adrPacked + 2] = 128;
					}
					else
					{
						padded[4 * adrPacked] = (unsigned char)((dataU[3 * adr] - 128) / len * 127.0f + 128.0f);
						padded[4 * adrPacked + 1] = (unsigned char)((dataU[3 * adr + 1] - 128) / len * 127.0f + 128.0f);
						padded[4 * adrPacked + 2] = (unsigned char)((dataU[3 * adr + 2] - 128) / len * 127.0f + 128.0f);
					}
				}
				else
				{
					len = sqrt(SQR(dataF[3 * adr]) + SQR(dataF[3 * adr + 1]) + SQR(dataF[3 * adr + 2]));
					if (len < EPS)
					{
						len = 0.0f;
						padded[4 * adrPacked] = 128;
						padded[4 * adrPacked + 1] = 128;
						padded[4 * adrPacked + 2] = 128;
					}
					else
					{
						padded[4 * adrPacked] = (unsigned char)(dataF[3 * adr] / len * 127.0 + 128.0);
						padded[4 * adrPacked + 1] = (unsigned char)(dataF[3 * adr + 1] / len * 127.0 + 128.0);
						padded[4 * adrPacked + 2] = (unsigned char)(dataF[3 * adr + 2] / len * 127.0 + 128.0);
					}
				}

				if (len > maxLen)
					maxLen = len;

				// store magnitude as float for higher precision
				magnitude[adrPacked] = len;
			}
		}
	}

	// adapt the range of the magnitude to [0,255]
	for (int z = 0; z<_vd->size[2]; ++z)
		for (int y = 0; y<_vd->size[1]; ++y)
			for (int x = 0; x<_vd->size[0]; ++x)
			{
				adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
				len = magnitude[adrPacked] / maxLen * UCHAR_MAX;
				padded[4 * adrPacked + 3] = (unsigned char)((len > 255.0f) ? 255 : ((len < 0.0f) ? 0 : len));
			}

	delete[] magnitude;

	return padded;
}

void* VectorDataSet::fillTexDataCharInterp()
{
	int size = _vd->texSize[0] * _vd->texSize[1] * _vd->texSize[2];
	int adr;
	int adrPacked;
	float len;
	float maxLen = -1.0;

	float *magnitude = new float[size];

	unsigned char *dataU = (unsigned char*)_vd->data;
	unsigned char *dataNextU = (unsigned char*)_vd->newData;
	float *dataF = (float*)_vd->data;
	float *dataNextF = (float*)_vd->newData;
	unsigned char *padded = new unsigned char[4 * size];

	memset(padded, 0, 4 * size * sizeof(char));

	// determine maximum length in the vector field
	// and normalize the direction
	// the original magnitude is stored in the forth channel/in *magnitude
	for (int z = 0; z<_vd->size[2]; ++z)
	{
		for (int y = 0; y<_vd->size[1]; ++y)
		{
			for (int x = 0; x<_vd->size[0]; ++x)
			{
				adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
				adr = (z*_vd->size[1] + y)*_vd->size[0] + x;

				unsigned char tdataU[3];
				float tdataF[3];
				if (_vd->dataType == DATRAW_UCHAR)
				{
					for (int idx = 0; idx < 3; idx++)
					{
						dataU[3 * adr + idx] -= 128;
						dataNextU[3 * adr + idx] -= 128;
						tdataU[idx] = dataU[3 * adr + idx] + (float)interpIndex / InterpSize * (dataNextU[3 * adr + idx] - dataU[3 * adr + idx]);
					}
					len = sqrt((float)SQR(tdataU[0]) + (float)SQR(tdataU[1]) + (float)SQR(tdataU[2]));
					if (len < EPS)
					{
						len = 0.0f;
						padded[4 * adrPacked] = 128;
						padded[4 * adrPacked + 1] = 128;
						padded[4 * adrPacked + 2] = 128;
					}
					else
					{
						padded[4 * adrPacked] = (unsigned char)((tdataU[0]) / len * 127.0f + 128.0f);
						padded[4 * adrPacked + 1] = (unsigned char)((tdataU[1]) / len * 127.0f + 128.0f);
						padded[4 * adrPacked + 2] = (unsigned char)((tdataU[2]) / len * 127.0f + 128.0f);
					}
				}
				else
				{
					for (int idx = 0; idx < 3; idx++)
					{
						tdataF[idx] = dataF[3 * adr + idx] + (float)interpIndex / InterpSize * (dataNextF[3 * adr + idx] - dataF[3 * adr + idx]);
					}
					len = sqrt(SQR(tdataF[0]) + SQR(tdataF[1]) + SQR(tdataF[2]));
					if (len < EPS)
					{
						len = 0.0f;
						padded[4 * adrPacked] = 128;
						padded[4 * adrPacked + 1] = 128;
						padded[4 * adrPacked + 2] = 128;
					}
					else
					{
						padded[4 * adrPacked] = (unsigned char)(tdataF[0] / len * 127.0 + 128.0);
						padded[4 * adrPacked + 1] = (unsigned char)(tdataF[1] / len * 127.0 + 128.0);
						padded[4 * adrPacked + 2] = (unsigned char)(tdataF[2] / len * 127.0 + 128.0);
					}
				}

				if (len > maxLen)
					maxLen = len;

				// store magnitude as float for higher precision
				magnitude[adrPacked] = len;
			}
		}
	}

	// adapt the range of the magnitude to [0,255]
	for (int z = 0; z<_vd->size[2]; ++z)
		for (int y = 0; y<_vd->size[1]; ++y)
			for (int x = 0; x<_vd->size[0]; ++x)
			{
				adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
				len = magnitude[adrPacked] / maxLen * UCHAR_MAX;
				padded[4 * adrPacked + 3] = (unsigned char)((len > 255.0f) ? 255 : ((len < 0.0f) ? 0 : len));
			}

	delete[] magnitude;

	return padded;
}


// --------------------------------------------------

VolumeDataSet::VolumeDataSet(void)
{
	_vd = new VolumeData;
}


VolumeDataSet::~VolumeDataSet(void)
{
	delete _vd;
}


bool VolumeDataSet::loadData(const char *fileName)
{
	float volSize[3];
	float maxVolSize = 0;
	int maxTexSize = 0;

	if (fileName)
		setFileName(fileName);
	else if (!_fileName)
	{
		fprintf(stderr, "VolumeData:  No filename set.\n");
		return false;
	}

	delete[](unsigned char*) _vd->data;
	_vd->data = NULL;
	_loaded = false;

	if (!_datFile.parseDatFile(_fileName))
	{
		fprintf(stderr, "VolumeData:  Could not parse DAT file \"%s\".\n",
			_fileName);
		return false;
	}

	// ensure that raw data consists only of scalar data
	if (_datFile.getDataDimension() != 1)
	{
		fprintf(stderr, "VolumeData:  DAT file \"%s\" refers not to a scalar data set.\n",
			_fileName);
		return false;
	}
	_vd->dataDim = 1; // scalar

					  // copy volume dimensions from DatFile to VolumeData
	_vd->dataType = _datFile.getDataType();

	if ((_vd->dataType != DATRAW_UCHAR) && (_vd->dataType != DATRAW_FLOAT))
	{
		fprintf(stderr, "VolumeData:  Only 8bit integer and 32bit "
			"float scalar is supported.\n");
		return false;
	}

	// determine 3D texture dimensions
	// and update slice distances
	for (int i = 0; i<3; ++i)
	{
		// data set dimensions
		_vd->size[i] = _datFile.getDataSizes()[i];
		// texture size
#if FORCE_POWER_OF_TWO_TEXTURE == 1
		_vd->texSize[i] = getNextPowerOfTwo(_vd->size[i]);
#else
		_vd->texSize[i] = _vd->size[i];
#endif
		// distance between slices
		_vd->sliceDist[i] = _datFile.getDataDists()[i];

		volSize[i] = _vd->size[i] * _vd->sliceDist[i];
		if (volSize[i] > maxVolSize)
			maxVolSize = volSize[i];
		if (_vd->texSize[i] > maxTexSize)
			maxTexSize = _vd->texSize[i];
	}
	// set up texture scaling and bounding box extents of the volume
	for (int i = 0; i < 3; ++i)
	{
		_vd->scale[i] = maxTexSize / (_vd->texSize[i] * _vd->sliceDist[i]);
		_vd->scaleInv[i] = _vd->texSize[i] * _vd->sliceDist[i] / maxTexSize;

		_vd->extent[i] = _vd->size[i] * _vd->sliceDist[i] / maxVolSize;
	}
	_vd->scale[3] = 0.0f;
	_vd->scaleInv[3] = 1.0f;

	_vd->center[0] = _vd->extent[0] / 2.0f;
	_vd->center[1] = _vd->extent[1] / 2.0f;
	_vd->center[2] = _vd->extent[2] / 2.0f;


	// read raw data
	_vd->data = _datFile.readRawData();

	if (!_vd->data)
	{
		fprintf(stderr, "VolumeData:  Could not read volume data set from RAW file \"%s\".\n",
			_fileName);
		return false;
	}

	_loaded = true;
	return true;
}


void VolumeDataSet::createTexture(const char *texName,
	GLuint texUnit,
	bool floatTex)
{
	GLuint texId;
	int size;
	int adr;
	int adrPacked;
	bool needPadding = false;
	void *paddedData = _vd->data;

	if (!_loaded)
		return;

	// create a texture id
	if (_tex.id == 0)
	{
		glGenTextures(1, &texId);
		_tex.setTex(GL_TEXTURE_3D, texId, texName);
	}
	_tex.texUnit = texUnit;
	_tex.width = _vd->texSize[0];
	_tex.height = _vd->texSize[1];
	_tex.depth = _vd->texSize[2];

	switch (_vd->dataType)
	{
	case DATRAW_UCHAR:
		_texSrcFmt = GL_UNSIGNED_BYTE;
		break;
	case DATRAW_FLOAT:
		_texSrcFmt = GL_FLOAT;
		break;
	default:
		break;
	}

#if FORCE_POWER_OF_TWO_TEXTURE == 1
	if ((_vd->texSize[0] != _vd->size[0])
		|| (_vd->texSize[1] != _vd->size[1])
		|| (_vd->texSize[2] != _vd->size[2]))
	{
		fprintf(stderr, "VolumeData:  Dimensions are not 2^n.\n");
		needPadding = true;
	}
#endif

	if (floatTex)
		_texIntFmt = GL_LUMINANCE16F_ARB;
	else
		_texIntFmt = GL_LUMINANCE;

	_tex.format = _texIntFmt;

	size = _vd->texSize[0] * _vd->texSize[1] * _vd->texSize[2];


	if (needPadding)
	{
		if (_vd->dataType == DATRAW_UCHAR)
		{
			paddedData = new unsigned char[size];

			// perform padding of the original data
			for (int z = 0; z<_vd->size[2]; ++z)
				for (int y = 0; y<_vd->size[1]; ++y)
					for (int x = 0; x<_vd->size[0]; ++x)
					{
						adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
						adr = (z*_vd->size[1] + y)*_vd->size[0] + x;
						((unsigned char*)paddedData)[adrPacked] =
							((unsigned char*)_vd->data)[adr];
					}
		}
		else
		{
			paddedData = new float[size];

			// perform padding of the original data
			for (int z = 0; z<_vd->size[2]; ++z)
				for (int y = 0; y<_vd->size[1]; ++y)
					for (int x = 0; x<_vd->size[0]; ++x)
					{
						adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
						adr = (z*_vd->size[1] + y)*_vd->size[0] + x;
						((float*)paddedData)[adrPacked] =
							((float*)_vd->data)[adr];
					}
		}
	}

	glBindTexture(GL_TEXTURE_3D, _tex.id);
	glTexImage3DEXT(GL_TEXTURE_3D, 0, _texIntFmt, _vd->texSize[0],
		_vd->texSize[1], _vd->texSize[2], 0, GL_LUMINANCE,
		_texSrcFmt, paddedData);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	CHECK_FOR_OGL_ERROR();

	if (needPadding)
	{
		if (_vd->dataType == DATRAW_UCHAR)
			delete[] static_cast<unsigned char*>(paddedData);
		else
			delete[] static_cast<float*>(paddedData);
		paddedData = NULL;
	}
}


void VolumeDataSet::normalizeData(void)
{
	if (!_vd->data)
		return;

	switch (_vd->dataType)
	{
	case DATRAW_UCHAR:
		normalizeDataChar();
		break;
	case DATRAW_FLOAT:
		normalizeDataFloat();
		break;
	default:
		break;
	}
}


void VolumeDataSet::normalizeDataFloat(void)
{
	float min = ((float*)_vd->data)[0];
	float max = -FLT_MAX;
	float *data = (float*)_vd->data;

	for (int i = 0; i<_vd->size[0] * _vd->size[1] * _vd->size[2]; ++i)
	{
		if (max < data[i])
			max = data[i];
		else if (min > data[i])
			min = data[i];
	}

	for (int i = 0; i<_vd->size[0] * _vd->size[1] * _vd->size[2]; ++i)
		data[i] = (data[i] - min) / (max - min);
}


void VolumeDataSet::normalizeDataChar(void)
{
	unsigned char min = ((unsigned char*)_vd->data)[0];
	unsigned char max = 0;
	unsigned char *data = (unsigned char*)_vd->data;

	for (int i = 0; i<_vd->size[0] * _vd->size[1] * _vd->size[2]; ++i)
	{
		if (max < data[i])
			max = data[i];
		else if (min > data[i])
			min = data[i];
	}

	for (int i = 0; i<_vd->size[0] * _vd->size[1] * _vd->size[2]; ++i)
		data[i] = (unsigned char)((data[i] - min) / (float)(max - min) * UCHAR_MAX);
}


// --------------------------------------------------

NoiseDataSet::NoiseDataSet(void) : _useGradient(true)
{
	_vd = new VolumeData;
}


NoiseDataSet::~NoiseDataSet(void)
{
	delete _vd;
}


bool NoiseDataSet::loadData(const char *fileName)
{
	int size = 0;
	int resNoise[3] = { 256, 256, 256 };

	if (fileName)
		setFileName(fileName);
	else if (!_fileName)
	{
		fprintf(stderr, "NoiseData:  No filename for noise data set given.\n");
		//return false;
	}

	delete[] static_cast<unsigned char*>(_vd->data);
	_vd->data = NULL;
	_loaded = false;

	// if noise was not loaded, generate white noise
	if (!loadRawData())
	{
		setFileName(NULL);

		_vd->size[0] = resNoise[0];
		_vd->size[1] = resNoise[1];
		_vd->size[2] = resNoise[2];
		fprintf(stdout, "NoiseData:  Using random noise  %dx%dx%d\n",
			_vd->size[0], _vd->size[1], _vd->size[2]);

		size = _vd->size[0] * _vd->size[1] * _vd->size[2];
		_vd->data = new unsigned char[size];

		_vd->dataType = DATRAW_UCHAR;
		_vd->dataDim = 1; // scalar

						  // generate white noise
		for (int i = 0; i<size; ++i)
		{
			((unsigned char*)_vd->data)[i] = ((int)floor(0.6f*rand() / (RAND_MAX + 1.0f) + 0.5f)) * 255;
		}
	}
	else
	{
		fprintf(stdout, "NoiseData:  Using precomputed noise  %dx%dx%d\n",
			_vd->size[0], _vd->size[1], _vd->size[2]);
	}

	_loaded = true;

	return true;
}


void NoiseDataSet::createTexture(const char *texName,
	GLuint texUnit,
	bool floatTex)
{
	GLuint texId;
	int size;
	int adr, adrPacked;
	bool needPadding = false;
	float *gradients = NULL;
	unsigned char *packedData = NULL;
	unsigned char *gradTmp = NULL;

	CHECK_FOR_OGL_ERROR();

	if (!_loaded)
		return;

	// create a texture id
	if (_tex.id == 0)
	{
		glGenTextures(1, &texId);
		_tex.setTex(GL_TEXTURE_3D, texId, texName);
	}
	_tex.texUnit = texUnit;

	if (floatTex)
		fprintf(stderr, "NoiseData:  Float textures are currently "
			"not supported.\n");



#if FORCE_POWER_OF_TWO_TEXTURE == 0
	_vd->texSize[0] = _vd->size[0];
	_vd->texSize[1] = _vd->size[1];
	_vd->texSize[2] = _vd->size[2];
#else
	_vd->texSize[0] = nextPowerTwo(_vd->size[0]);
	_vd->texSize[1] = nextPowerTwo(_vd->size[1]);
	_vd->texSize[2] = nextPowerTwo(_vd->size[2]);

	if ((_vd->texSize[0] != _vd->size[0])
		|| (_vd->texSize[1] != _vd->size[1])
		|| (_vd->texSize[2] != _vd->size[2]))
	{
		needPadding = true;
	}
#endif

	_tex.width = _vd->texSize[0];
	_tex.height = _vd->texSize[1];
	_tex.depth = _vd->texSize[2];

	size = _vd->texSize[0] * _vd->texSize[1] * _vd->texSize[2];

	if (_useGradient)
	{
		_texSrcFmt = GL_UNSIGNED_BYTE;
		_texIntFmt = GL_RGBA; // because of noise+gradient

		packedData = new unsigned char[4 * size];
		memset(packedData, 0, 4 * size * sizeof(char));

		// try to load precomputed gradients if volume was loaded earlier
		if (_loaded)
			gradTmp = (unsigned char*)loadGradients(_vd, _fileName, DATRAW_UCHAR);
		if (!gradTmp) // gradients not loaded from file
		{
			// compute gradients
			gradients = computeGradients(_vd);
			if (!gradients)
			{
				fprintf(stderr, "NoiseData:  Error during gradient computation.\n");
				return; // TODO: right?
			}
			filterGradients(_vd, gradients);
			// quantize gradients to 8bit
			gradTmp = (unsigned char*)quantizeGradients(_vd, gradients, DATRAW_UCHAR);

			// store gradients for the next time
			// but don't save gradients of white noise
			if (_fileName)
				if (!saveGradients(_vd, _fileName, gradTmp, DATRAW_UCHAR))
					fprintf(stderr, "NoiseData:  Saving gradients was "
						"not sucessful.\n");

		}
		else
		{
			fprintf(stdout, "NoiseData:  Using stored gradients "
				"from \"%s%s\".\n", _fileName, GRADIENTS_EXT);
			// TODO: adapt to float noise?
		}

		// pack scalar noise and gradients
		for (int z = 0; z<_vd->size[2]; ++z)
			for (int y = 0; y<_vd->size[1]; ++y)
				for (int x = 0; x<_vd->size[0]; ++x)
				{
					adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
					adr = (z*_vd->size[1] + y)*_vd->size[0] + x;
					assert(adr == adrPacked);
					packedData[4 * adrPacked + 0] = gradTmp[3 * adr];
					packedData[4 * adrPacked + 1] = gradTmp[3 * adr + 1];
					packedData[4 * adrPacked + 2] = gradTmp[3 * adr + 2];
					packedData[4 * adrPacked + 3] = ((unsigned char*)_vd->data)[adr];
				}

		glBindTexture(GL_TEXTURE_3D, _tex.id);
		glTexImage3D(GL_TEXTURE_3D, 0, _texIntFmt, _vd->texSize[0],
			_vd->texSize[1], _vd->texSize[2], 0, GL_RGBA,
			_texSrcFmt, packedData);

		delete[] packedData;
		packedData = NULL;
	}
	else
	{
		_texIntFmt = GL_LUMINANCE; // because of noise
		_texSrcFmt = GL_UNSIGNED_BYTE;

		if (needPadding)
		{
			packedData = new unsigned char[4 * size];

			// perform padding of the original data
			for (int z = 0; z<_vd->size[2]; ++z)
				for (int y = 0; y<_vd->size[1]; ++y)
					for (int x = 0; x<_vd->size[0]; ++x)
					{
						adrPacked = (z*_vd->texSize[1] + y)*_vd->texSize[0] + x;
						adr = (z*_vd->size[1] + y)*_vd->size[0] + x;
						packedData[adrPacked] = ((unsigned char*)_vd->data)[adr];
					}
		}
		else
		{
			packedData = (unsigned char*)_vd->data;
		}

		glBindTexture(GL_TEXTURE_3D, _tex.id);
		glTexImage3D(GL_TEXTURE_3D, 0, _texIntFmt, _vd->texSize[0],
			_vd->texSize[1], _vd->texSize[2], 0, GL_LUMINANCE,
			_texSrcFmt, packedData);

		if (needPadding)
		{
			delete[] packedData;
			packedData = NULL;
		}
	}
	_tex.format = _texIntFmt;

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	CHECK_FOR_OGL_ERROR();

	delete[] gradients;
	delete[] gradTmp;
}


bool NoiseDataSet::loadRawData(void)
{
	FILE *src = NULL;
	int size;

	// try to open the noise file
	src = fopen(_fileName, "rb");
	if (!src)
	{
		fprintf(stderr, "NoiseData:  Could not load noise from "
			"(\"%s\").\n", _fileName);
		return false;
	}

	// read "header"
	if (fread((void*)_vd->size, 4, 3, src) != 3)
	{
		fprintf(stderr, "NoiseData:  Could not read noise header from "
			"\"%s\".\n", _fileName);
		fclose(src);
		return false;
	}

	// read noise data
	size = _vd->size[0] * _vd->size[1] * _vd->size[2];
	_vd->data = new unsigned char[size];
	_vd->dataType = DATRAW_UCHAR;
	_vd->dataDim = 1; // scalar

	if (fread(_vd->data, _vd->size[0] * _vd->size[1],
		_vd->size[2], src) != static_cast<unsigned int>(_vd->size[2]))
	{
		fprintf(stderr, "NoiseData:  Error reading noise data from "
			"\"%s\".\n", _fileName);
		fclose(src);
		delete[] static_cast<unsigned char*>(_vd->data);
		_vd->data = NULL;
		return false;
	}
	fclose(src);

	return true;
}


// --------------------------------------------------

LICFilter::LICFilter(void) : _filterWidth(0), _filterData(NULL)
{
}


LICFilter::~LICFilter(void)
{
	delete[] _filterData;
}


void LICFilter::createBoxFilter(unsigned int width)
{
	_filterWidth = nextPowerTwo(width);
	_filterData = new unsigned char[_filterWidth];

	memset(_filterData, 255, _filterWidth * sizeof(unsigned char));

	_invFilterArea = 0.5f;
}


bool LICFilter::loadData(const char *fileName)
{
	int shift;
	Image img;

	delete[] _filterData;
	_filterData = NULL;
	_loaded = false;

	if (fileName)
		setFileName(fileName);
	else if (!_fileName)
	{
		fprintf(stderr, "FilterKernel:  No filename set.\n");
		return false;
	}

	// try to read filter kernel stored in png

	if (!pngRead(_fileName, &img))
	{
		fprintf(stderr, "FilterKernel:  Could not load filter kernel "
			"(\"%s\").\n", _fileName);
		return false;
	}

	if (img.channel > 1)
	{
		fprintf(stderr, "FilterKernel:  \"%s\" contains more than one channel. "
			"Using first one.\n", _fileName);
	}

	_filterWidth = nextPowerTwo(img.width);
	shift = (_filterWidth - img.width) / 2;

	fprintf(stdout, "FilterKernel:  Filter has width %d. Using %d.\n",
		img.width, _filterWidth);

	_filterData = new unsigned char[_filterWidth];
	memset(_filterData, 0, _filterWidth * sizeof(unsigned char));
	for (int i = 0; i<img.width; ++i)
	{
		_filterData[i + shift] = img.imgData[i*img.channel];
	}

	_invFilterArea = calcFilterKernelInvArea();
	_texSrcFmt = GL_UNSIGNED_BYTE;
	_loaded = true;

	delete[] img.imgData;
	return true;
}


void LICFilter::createTexture(const char *texName,
	GLuint texUnit,
	bool floatTex)
{
	GLuint texId;

	if (!_filterData)
		return;

	if (_tex.id == 0)
	{
		glGenTextures(1, &texId);
		_tex.setTex(GL_TEXTURE_1D, texId, texName);
	}
	_tex.texUnit = texUnit;
	_tex.width = _filterWidth;

	_texIntFmt = GL_LUMINANCE;
	_tex.format = _texIntFmt;

	glBindTexture(GL_TEXTURE_1D, _tex.id);

	glTexImage1D(GL_TEXTURE_1D, 0, _texIntFmt, _filterWidth,
		0, GL_LUMINANCE, _texSrcFmt, _filterData);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
}


float LICFilter::calcFilterKernelInvArea(void)
{
	int i;
	float area = 0.0f;

	for (i = 0; i<_filterWidth; ++i)
		area += _filterData[i];

	return 0.5f*_filterWidth * 255.0f / area;
}

