#ifndef _DATASET_H_
#define _DATASET_H_

#ifdef _WIN32
#  include <windows.h>
#endif

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "texture.h"
#include "mmath.h"
#include "reader.h"
#include "types.h"
#include <vector>
#include <deque>


struct VolumeData
{
	VolumeData(void) : data(NULL), dataDim(1), dataType(DATRAW_NONE)
	{
		sliceDist[0] = sliceDist[1] = sliceDist[2] = 1.0f;
		size[0] = size[1] = size[2] = 1;
		texSize[0] = texSize[1] = texSize[2] = 1;
		extent[0] = extent[1] = extent[2] = 1.0f;
		scale[0] = scale[1] = scale[2] = scale[3] = 1.0f;
		scaleInv[0] = scaleInv[1] = scaleInv[2] = scaleInv[3] = 1.0f;
		center[0] = center[1] = center[2] = 0.0f;
	}
	VolumeData(const VolumeData& obj)
		:data(obj.data), newData(obj.newData),
		dataDim(obj.dataDim),dataType(obj.dataType)
	{
		sliceDist[0] = obj.sliceDist[0];
		sliceDist[1] = obj.sliceDist[1];
		sliceDist[2] = obj.sliceDist[2];

		size[0] = obj.size[0];
		size[1] = obj.size[1];
		size[2] = obj.size[2];

		texSize[0] = obj.texSize[0];
		texSize[1] = obj.texSize[1];
		texSize[2] = obj.texSize[2];

		extent[0] = obj.extent[0]; extent[1] = obj.extent[1]; extent[2] = obj.extent[2];
		scale[0] = obj.scale[0]; scale[1] = obj.scale[1]; scale[2] = obj.scale[2]; scale[3] = obj.scale[3];
		scaleInv[0] = obj.scaleInv[0]; scaleInv[1] = obj.scaleInv[1]; scaleInv[2] = obj.scaleInv[2]; scaleInv[3] = obj.scaleInv[3];
		center[0] = obj.center[0]; center[1] = obj.center[1]; center[2] = obj.center[2];
	}
	~VolumeData(void);

	void *data;
	void *newData;
	//std::vector<void*> dataSets;
	unsigned char dataDim;
	DataType dataType;

	float sliceDist[3];
	int size[3];

	int texSize[3];

	float extent[3];
	float scale[4];
	float scaleInv[4];

	float center[3];
};


class DataSet
{
public:
	DataSet(void);
	virtual ~DataSet(void);

	void setFileName(const char *fileName);
	const char* getFileName(void) { return _fileName; }
	bool isLoaded(void) { return _loaded; }

	// load data set and return status if sucessful
	// the data is stored in _vd
	// should set _loaded and _fileName
	virtual bool loadData(const char *fileName = NULL) = 0;

	virtual void createTexture(const char *texName,
		GLuint texUnit = GL_TEXTURE0_ARB,
		bool floatTex = false) = 0;
	Texture* getTextureRef(void) { return &_tex; }
	const Texture getTexture(void) { return _tex; }

protected:
	char *_fileName;
	bool _loaded;

	Texture _tex;

	GLint  _texIntFmt;
	GLenum _texSrcFmt;

private:
};


class VectorDataSet : public DataSet
{
public:
	VectorDataSet(void);
	virtual ~VectorDataSet(void);

	// parses dat file but does not load any data into _vd->data
	virtual bool loadData(const char *fileName = NULL);

	// load a single time step and return the data
	// (data is not stored in VolumeData struct)
	void* loadTimeStep(int timeStep);

	VolumeData* getVolumeData(void) { return _vd; }
	std::deque<VolumeData*>& getVolumeDataSet(void) { return _volumeSet; }
	std::vector<Texture*>& getTextureSet(void) { return _texSet; }

	// updates data pointer of VolumeData
	// the memory of the previous reference is not freed!
	void setDataPointer(void *dataPtr) { _vd->data = dataPtr; }

	// data pointer has to be set before with setDataPointer()
	virtual void createTexture(const char *texName,
		GLuint texUnit = GL_TEXTURE0_ARB,
		bool floatTex = false);

	void createTextureIterp(const char *texName,
		GLuint texUnit = GL_TEXTURE0_ARB,
		bool floatTex = false);

	Texture * createTextures(int index, const char *texName,
		GLuint texUnit = GL_TEXTURE0_ARB,
		bool floatTex = false);

	const int getTimeStepBegin(void) { return _datFile.getTimeStepBegin(); }
	const int getTimeStepEnd(void) { return _datFile.getTimeStepEnd(); }
	int getNextTimeStep(void);
	int NextTimeStep(void);
	int getCurTimeStep(void);

	Texture* getTextureSetRef(int index) { return _texSet[index]; }

	int checkInterpolateStage();
	void setInterpolateSize(int size) { InterpSize = size; };
	void increaseInterpIndex() { interpIndex++; }

	void setVolumeDataIndex(int i)
	{
		_vd = _volumeSet[i];
	}

protected:
private:
	// fill a zero padded texture with normalized vector direction 
	// in rgb and the magnitude in a 
	void* fillTexDataFloat(int i = 0);
	void* fillTexDataFloatInterp();
	// fill a zero padded texture with normalized vector direction 
	// in rgb and the magnitude in a
	void* fillTexDataChar(int i = 0);
	void* fillTexDataCharInterp();

	VolumeData *_vd;
	DatFile _datFile;

	std::deque<VolumeData*> _volumeSet;

	//preload sequence of texture
	std::vector<Texture*> _texSet;

	//Interpolate step Index
	int interpIndex;
	int InterpSize;
};


class VolumeDataSet : public DataSet
{
public:
	VolumeDataSet(void);
	virtual ~VolumeDataSet(void);

	virtual bool loadData(const char *fileName = NULL);

	VolumeData* getVolumeData(void) { return _vd; }

	virtual void createTexture(const char *texName,
		GLuint texUnit = GL_TEXTURE0_ARB,
		bool floatTex = false);

protected:
	inline void normalizeData(void);

private:
	void normalizeDataFloat(void);
	void normalizeDataChar(void);

	VolumeData *_vd;
	DatFile _datFile;
};


class NoiseDataSet : public DataSet
{
public:
	NoiseDataSet(void);
	virtual ~NoiseDataSet(void);

	void enableGradient(bool enable) { _useGradient = enable; }
	bool isGradientEnabled(void) { return _useGradient; }

	virtual bool loadData(const char *fileName = NULL);

	VolumeData* getVolumeData(void) { return _vd; }

	virtual void createTexture(const char *texName,
		GLuint texUnit = GL_TEXTURE0_ARB,
		bool floatTex = false);

protected:
	bool loadRawData(void);

private:
	VolumeData *_vd;

	bool _useGradient;
};


class LICFilter : public DataSet
{
public:
	LICFilter(void);
	virtual ~LICFilter(void);

	const char* getFileName(void) { return _fileName; }
	bool isLoaded(void) { return _loaded; }

	void createBoxFilter(unsigned int width = 256);
	// load data set and return status if sucessful
	// the data is stored in _vd
	// should set _loaded and _fileName
	virtual bool loadData(const char *fileName = NULL);

	virtual void createTexture(const char *texName,
		GLuint texUnit = GL_TEXTURE0_ARB,
		bool floatTex = false);

	unsigned char* getFilterData(void) { return _filterData; }

	int getFilterWidth(void) { return _filterWidth; }
	float getInverseFilterArea(void) { return _invFilterArea; }


protected:
	float calcFilterKernelInvArea(void);

private:
	int _filterWidth;
	unsigned char *_filterData;
	float _invFilterArea;
};


#endif // _DATASET_H_
