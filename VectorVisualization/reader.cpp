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
 * Filename: reader.cpp
 *
 * $Id: reader.cpp,v 1.3 2008/05/13 08:55:31 falkmn Exp $
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "reader.h"
#include "types.h"



int getDataTypeSize(DataType t)
{
    switch (t)
    {
    case DATRAW_UCHAR:
        return 1;
        break;
    case DATRAW_USHORT:
        return 2;
        break;
    case DATRAW_FLOAT:
        return 4;
        break;
    default:
        fprintf(stderr, "Unsupported data type!\n");
        break;
    }
    return 0;
}


DatFile::DatFile(void)
{
    _datFileName = NULL;
    _rawFileName = NULL;

    _dataType = DATRAW_NONE;

    _sizes[0] = _sizes[1] = _sizes[2] = 0;
    _dists[0] = _dists[1] = _dists[2] = 1.0f;
	_timestep = _timeStepBeg;
}


DatFile::~DatFile(void)
{
    delete [] _datFileName;
    delete [] _rawFileName;
}


bool DatFile::parseDatFile(char *datFileName)
{
    char *cp, line[255], rawFileName[255];
    char tmp[10];
    int parseError = 0;
    FILE *fp;

    delete [] _datFileName;
    delete [] _rawFileName;
    _datFileName = NULL;
    _rawFileName = NULL;

    if (!datFileName)
        return false;

    if (! (fp = fopen(datFileName, "rb")))
    {
        perror("opening .dat file failed");
        return false;
    }

    // store filename
    _datFileName = new char[strlen(datFileName)+1];
    strcpy(_datFileName, datFileName);

    _sizes[0] = _sizes[1] = _sizes[2] = 0;
    _dists[0] = _dists[1] = _dists[2] = 1.0f;

    _timeStepBeg = 0;
    _timeStepEnd = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] != '#')
        {
            if (strstr(line, "ObjectFileName"))
            {
                if (! (cp = strchr(line, ':')))
                {
                    parseError = 1;
                    break;
                }
                if (sscanf(cp + 1, "%s", rawFileName) != 1)
                {
                    parseError = 1;
                    break;
                }
                if (!rawFileName)
                    return false;
                _rawFileName = new char[strlen(rawFileName)+1];
                strcpy(_rawFileName, rawFileName);
            }
            else if (strstr(line, "Resolution"))
            {
                if (! (cp = strchr(line, ':')))
                {
                    parseError = 1;
                    break;
                }
                if (sscanf(cp + 1, "%i %i %i",
                    &_sizes[0], &_sizes[1], &_sizes[2]) != 3)
                {
                        parseError = 1;
                        break;
                }
            }
            else if (strstr(line, "SliceThickness"))
            {
                if (! (cp = strchr(line, ':')))
                {
                    parseError = 1;
                    break;
                }
                if (sscanf(cp + 1, "%f %f %f",
                    &_dists[0], &_dists[1], &_dists[2]) != 3)
                {
                        parseError = 1;
                        break;
                }
            }
            else if (strstr(line, "Format"))
            {
                if ((cp = strstr(line, "UCHAR")))
                {
                    _dataType = DATRAW_UCHAR;
                    strcpy(tmp, "char");
                }
                else if ((cp = strstr(line, "USHORT")))
                {
                    _dataType = DATRAW_USHORT;
                    strcpy(tmp, "short");
                }
                else if ((cp = strstr(line, "FLOAT")))
                {
                    _dataType = DATRAW_FLOAT;
                    strcpy(tmp, "float");
                }
                else
                {
                    fprintf(stderr, "DatFile:  Cannot process data other than of "
                        "UCHAR*, USHORT* and FLOAT* format.\n");
                    return false;
                }
                parseDataDim(cp);
                fprintf(stdout, "Data Format:   %d x %s\n", _dataDim, tmp);
            }
            else if (strstr(line, "TimeDependent"))
            {
                if (! (cp = strchr(line, ':')))
                {
                    parseError = 1;
                    break;
                }
                if (sscanf(cp+1, "%i %i",&_timeStepBeg, &_timeStepEnd) != 2)
                {

                    parseError = 1;
                    break;
                }
                if ((_timeStepBeg < 0) || (_timeStepEnd < _timeStepBeg))
                {
                    fprintf(stderr, "DatFile:  Illegal boundaries for Timedependent Data Set "
                        "(%d-%d).\n", _timeStepEnd, _timeStepBeg);
                    return false;
                }
				_timestep = _timeStepBeg;
            }
            else
            {
                fprintf(stderr, "DatFile:  Skipping line %s", line);
            }
        }
    }

    fclose(fp);
    if (parseError)
    {
        fprintf(stderr, "parse error: %s\n", line);
        return false;
    }

    // test whether raw files exist
    snprintf(rawFileName, 255, _rawFileName, _timeStepBeg);

    if (! (fp = fopen(rawFileName, "rb")))
    {
        strcpy(line, _datFileName);
        if (! (cp = strrchr(line, DIR_SEP)) && ! (cp = strrchr(line, DIR_SEP_WIN)))
        {
            fprintf(stderr, "DatFile:  No valid search path for RAW file \"%s\".\n",
                    _rawFileName);
            return false;
        }
        strcpy(cp + 1, _rawFileName);
        delete [] _rawFileName;
        _rawFileName = new char[strlen(line)+1];
        strcpy(_rawFileName, line);

        snprintf(rawFileName, 255, _rawFileName, _timeStepBeg);

        if (! (fp = fopen(rawFileName, "rb")))
        {
            fprintf(stderr, "DatFile:  Could not open RAW file \"%s\".\n", rawFileName);
            return false;
        }
    }
    fclose(fp);

    // check for all timesteps except first
    for (int i=_timeStepBeg+1; i<=_timeStepEnd; ++i)
    {
        snprintf(rawFileName, 255, _rawFileName, i);

        if (! (fp = fopen(rawFileName, "rb")))
        {
            fprintf(stderr, "DatFile:  Could not open RAW file \"%s\" for timestep %d.\n",
                    rawFileName, i);
            return false;
        }
        fclose(fp);
    }

    return true;
}


void* DatFile::readRawData(int timeStep)
{
    size_t size;
    std::ifstream in;
    int dataTypeSize;
    char rawFileName[255];
    void *data = NULL;

    // check for boundaries
    if ((timeStep < _timeStepBeg) || (timeStep > _timeStepEnd))
    {
        return NULL;
    }

    snprintf(rawFileName, 255, _rawFileName, timeStep);

    in.open(rawFileName, std::ios::in | std::ios::binary);
    if (!in.is_open())
    {
        fprintf(stderr, "Could not open RAW file. No file \"%s\".\n",
                rawFileName);
            return NULL;
    }

    dataTypeSize = getDataTypeSize(_dataType);
    size = _dataDim * _sizes[0] * _sizes[1] * _sizes[2];

    data = new char[dataTypeSize * size];

    in.read((char*)data, static_cast<std::streamsize>(dataTypeSize*size));
    if (in.fail())
    {
        fprintf(stderr, "Reading volume data \"%s\" failed.\n",
                _rawFileName);
        in.clear();
    }
    in.close();

    return data;
}


void DatFile::parseDataDim(char *line)
{
    char *cp = line;

    while (*cp && ! isdigit(*cp))
    {
        ++cp;
    }
    if (*cp)
    {
        if (sscanf(cp, "%i", &_dataDim) != 1)
            _dataDim = 1;
    }
    else
    {
        _dataDim = 1;
    }
}

int DatFile::getNextTimeStep()
{
	_timestep = ( _timestep == _timeStepEnd ) ? _timeStepBeg : _timestep + 1;
	return _timestep;
}

int DatFile::NextTimeStep()
{
	return (_timestep == _timeStepEnd) ? _timeStepBeg : _timestep + 1;
}