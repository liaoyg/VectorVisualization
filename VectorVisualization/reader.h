/*
 * Copyright (c) 2008  Martin Falk <falk@visus.uni-stuttgart.de>
 *                     Visualization Research Center (VISUS), 
 *                     Universität Stuttgart, Germany
 *
 * This source code is distributed as part of the publication 
 * "Output-Sensitive 3D Line Integral Convolution". 
 * Sample images and movies of this application can be found 
 * at http://www.vis.uni-stuttgart.de/texflowvis . 
 * This file may be distributed, modified, and used free of charge 
 * as long as this copyright notice is included in its original 
 * form. Commercial use is strictly prohibited. However we request
 * that acknowledgement is given to the following article
 *
 *     M. Falk, D. Weiskopf.
 *     Output-Sensitive 3D Line Integral Convolution,
 *     IEEE Transactions on Visualization and Computer Graphics, 2008.
 *
 * This code is based on the Single Pass Volume Renderer from Stegmaier et al.
 *     S. Stegmaier, M. Strengert, T. Klein, and T. Ertl. 
 *     A Simple and Flexible Volume Rendering Framework for 
 *     Graphics-Hardware-based Raycasting, 
 *     Proceedings of Volume Graphics 2005, pp.187-195, 2005
 *
 * Filename: reader.h
 * 
 * $Id: reader.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _READER_H_
#define _READER_H_

enum DataType { DATRAW_NONE, DATRAW_UCHAR, DATRAW_USHORT, DATRAW_FLOAT };

int getDataTypeSize(DataType t);


class DatFile
{
public:

    DatFile(void);
    ~DatFile(void);

    bool parseDatFile(char *datFileName);
    
    void* readRawData(int timeStep=0);

    const char* getDatFileName(void) { return _datFileName; }
    const char* getRawFileName(void) { return _rawFileName; }

    DataType getDataType(void) { return _dataType; }
    const int* getDataSizes(void) { return _sizes; }
    const float* getDataDists(void) { return _dists; }
    const int getDataDimension(void) { return _dataDim; }
    const int getTimeStepBegin(void) { return _timeStepBeg; }
    const int getTimeStepEnd(void) { return _timeStepEnd; }
	int getNextTimeStep();
	int NextTimeStep();
	int getCurTimeStep(void) { return _timestep; }

protected:
    void parseDataDim(char *line);

private:

    char *_datFileName;
    char *_rawFileName;

    DataType _dataType;
    int _sizes[3];
    float _dists[3];
    int _dataDim;
    int _timeStepBeg;
    int _timeStepEnd; // == timeStepBeg if only single timestep
	int _timestep;
};

#endif // _READER_H_
