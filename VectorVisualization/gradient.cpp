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
 * Filename: gradient.cpp
 * 
 * $Id: gradient.cpp,v 1.2 2008/05/09 08:23:24 falkmn Exp $ 
 */

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <limits.h>
#include "dataSet.h"
#include "gradient.h"
#include "types.h"


float getVoxel(VolumeData *vd, int x, int y, int z);
inline unsigned char getVoxel8(VolumeData *vd, int x, int y, int z);
inline unsigned short getVoxel16(VolumeData *vd, int x, int y, int z);
inline float getVoxel32f(VolumeData *vd, int x, int y, int z);

// append corresponding extension to fileName and create a new string
char* createGradientsFileName(char *fileName, bool useFloat=false);

void quantize8(float *grad, unsigned char *data);
void quantize16(float *grad, unsigned short *data);


float getVoxel(VolumeData *vd, int x, int y, int z)
{
    switch (vd->dataType) {
    case DATRAW_UCHAR:
        return (float)getVoxel8(vd, x, y, z);
        break;
    case DATRAW_USHORT:
        return (float)getVoxel16(vd, x, y, z);
        break;
    case DATRAW_FLOAT:
        return getVoxel32f(vd, x, y, z);
    default:
        fprintf(stderr, "Calculating Gradients:  Unsupported data type.\n");
        exit(1);
        break;
    }
    return 0.0f;
}


unsigned char getVoxel8(VolumeData *vd, int x, int y, int z)
{
    return ((unsigned char*)vd->data)
        [(z*vd->size[1]+y)*vd->size[0]+x];
}


unsigned short getVoxel16(VolumeData *vd, int x, int y, int z)
{
    return ((unsigned short*)vd->data)
        [(z*vd->size[1]+y)*vd->size[0]+x];
}


float getVoxel32f(VolumeData *vd, int x, int y, int z)
{
    return ((float*)vd->data)[(z*vd->size[1]+y)*vd->size[0]+x];
}


char* createGradientsFileName(char *fileName, bool useFloat)
{
    char *buf;

    if (!fileName)
        return NULL;

    buf = new char[strlen(fileName) + 1
                   + MAX(strlen(GRADIENTS_EXT),strlen(GRADIENTSFLT_EXT))];

    strcpy(buf, fileName);

    if (useFloat)
        strcat(buf, GRADIENTSFLT_EXT);
    else
        strcat(buf, GRADIENTS_EXT);

    return buf;
}


void* loadGradients(VolumeData *vd, char *fileName, DataType dataType, bool useFloat)
{
    int size;
    std::ifstream in;
    char *buf;
    void *gradients = NULL;

    if (!fileName)
        return NULL;

    buf = createGradientsFileName(fileName, useFloat);

    in.open(buf, std::ios::in | std::ios::binary);
    if (!in.is_open())
    {
        fprintf(stderr, "loadGradients: No pre-computed gradients found.\n");
        delete [] buf;
        return NULL;
    }

    size = 3 * vd->size[0] * vd->size[1] * vd->size[2]
        * getDataTypeSize(dataType);
    gradients = new unsigned char[size];
    in.read((char*)gradients, static_cast<std::streamsize>(size));

    if (in.fail())
    {
        fprintf(stderr, "loadGradients: Reading gradients from \"%s\" failed.\n",
                buf);
        delete [] buf;
        in.close();
        return NULL;
    }
    in.close();
    delete [] buf;

    return gradients;
}


bool saveGradients(VolumeData *vd, char *fileName, 
                   void *gradients, DataType dataType, 
                   bool useFloat)
{
    int size;
    std::ofstream out;
    char *buf;

    buf = createGradientsFileName(fileName, useFloat);
    
    out.open(buf, std::ios::out | std::ios::binary);

    if (!out.is_open())
    {
        fprintf(stderr, "saveGradients: Could not open file \"%s\".\n",
                buf);
        delete [] buf;
        return false;
    }

    size = 3 * vd->size[0] * vd->size[1] * vd->size[2] * getDataTypeSize(dataType);
    out.write((char*)gradients, static_cast<std::streamsize>(size));

    if (out.fail())
    {
        fprintf(stderr, "saveGradients: Writing gradients to \"%s\" failed.\n", 
                buf);
        delete [] buf;
        out.close();
        return false;
    }
    delete [] buf;
    out.close();

    return true;
}


float* computeGradients(VolumeData *vd)
{
#if SOBEL == 1
    int i, j, k, dir;
#endif
    int di, vdi;
    float *gp = NULL;
    float *gradients = NULL;

    static int weights[][3][3][3] = {
        {{{-1, -3, -1},
          {-3, -6, -3},
          {-1, -3, -1}},
         {{ 0,  0,  0},
          { 0,  0,  0},
          { 0,  0,  0}},
         {{ 1,  3,  1},
          { 3,  6,  3},
          { 1,  3,  1}}},
        {{{-1, -3, -1},
          { 0,  0,  0},
          { 1,  3,  1}},
         {{-3, -6, -3},
          { 0,  0,  0},
          { 3,  6,  3}},
         {{-1, -3, -1},
          { 0,  0,  0},
          { 1,  3,  1}}},
        {{{-1,  0,  1},
          {-3,  0,  3},
          {-1,  0,  1}},
         {{-3,  0,  3},
          {-6,  0,  6},
          {-3,  0,  3}},
         {{-1,  0,  1},
          {-3,  0,  3},
          {-1,  0,  1}}}
    };

    fprintf(stderr, "Computing gradients ... may take a while\n");

    gradients = new float[3*vd->size[0]*vd->size[1]*vd->size[2]];
    gp = gradients;

    di = 0;
    vdi = 0;
    for (int idz = 0; idz < vd->size[2]; ++idz)
    {
        for (int idy = 0; idy < vd->size[1]; ++idy) 
        {
            for (int idx = 0; idx < vd->size[0]; ++idx) 
            {
#if SOBEL == 1
                if (idx > 0 && idx < vd->size[0] - 1 &&
                    idy > 0 && idy < vd->size[1] - 1 &&
                    idz > 0 && idz < vd->size[2] - 1) 
                {
                    for (dir = 0; dir < 3; ++dir) 
                    {
                        gp[dir] = 0.0f;
                        for (i = -1; i < 2; i++) 
                        {
                            for (j = -1; j < 2; j++) 
                            {
                                for (k = -1; k < 2; k++) 
                                {
                                    gp[dir] += weights[dir][i + 1][j + 1][k + 1]
                                        * getVoxel(vd, idx + i, idy + j, idz + k);
                                }
                            }
                        }
                        gp[dir] /= 2.0f * vd->sliceDist[dir];
                    }
                } 
                else 
                {
                    /* X-direction */
                    if (idx < 1)
                    {
                        gp[0] = (getVoxel(vd, idx + 1, idy, idz) -
                                 getVoxel(vd, idx, idy, idz))/
                            vd->sliceDist[0];
                    }
                    else 
                    {
                        gp[0] = (getVoxel(vd, idx, idy, idz) -
                                 getVoxel(vd, idx - 1, idy, idz))/
                            vd->sliceDist[0];
                    }

                    /* Y-direction */
                    if (idy < 1) 
                    {
                        gp[1] = (getVoxel(vd, idx, idy + 1, idz) -
                                 getVoxel(vd, idx, idy, idz))/
                            vd->sliceDist[1];
                    }
                    else
                    {
                        gp[1] = (getVoxel(vd, idx, idy, idz) -
                                 getVoxel(vd, idx, idy - 1, idz))/
                            vd->sliceDist[1];
                    }

                    /* Z-direction */
                    if (idz < 1) 
                    {
                        gp[2] = (getVoxel(vd, idx, idy, idz + 1) -
                                 getVoxel(vd, idx, idy, idz))/
                            vd->sliceDist[2];
                    }
                    else 
                    {
                        gp[2] = (getVoxel(vd, idx, idy, idz) -
                                 getVoxel(vd, idx, idy, idz - 1))/
                            vd->sliceDist[2];
                    }
                }
#else
                /* X-direction */
                if (idx < 1) 
                {
                    gp[0] = (getVoxel(vd, idx + 1, idy, idz) -
                             getVoxel(vd, idx, idy, idz))/
                        vd->sliceDist[0];
                } 
                else if (idx > vd->size[0] - 1) 
                {
                    gp[0] = (getVoxel(vd, idx, idy, idz) -
                             getVoxel(vd, idx - 1, idy, idz))/
                        vd->sliceDist[0];
                }
                else 
                {
                    gp[0] = (getVoxel(vd, idx + 1, idy, idz) -
                             getVoxel(vd, idx - 1, idy, idz))/
                        vd->sliceDist[0] * 0.5f;
                }

                /* Y-direction */
                if (idy < 1) 
                {
                    gp[1] = (getVoxel(vd, idx, idy + 1, idz) -
                             getVoxel(vd, idx, idy, idz))/
                        vd->sliceDist[1];
                }
                else if (idy > vd->size[1] - 1)
                {
                    gp[1] = (getVoxel(vd, idx, idy, idz) -
                             getVoxel(vd, idx, idy - 1, idz))/
                        vd->sliceDist[1];
                } 
                else 
                {
                    gp[1] = (getVoxel(vd, idx, idy + 1, idz) -
                             getVoxel(vd, idx, idy - 1, idz))/
                        vd->sliceDist[1] * 0.5f;
                }

                /* Z-direction */
                if (idz < 1)
                {
                    gp[2] = (getVoxel(vd, idx, idy, idz + 1) -
                             getVoxel(vd, idx, idy, idz))/
                        vd->sliceDist[2];
                } 
                else if (idz > vd->size[2] - 1) 
                {
                    gp[2] = (getVoxel(vd, idx, idy, idz) -
                             getVoxel(vd, idx, idy, idz - 1))/
                        vd->sliceDist[2];
                } 
                else 
                {
                    gp[2] = (getVoxel(vd, idx, idy, idz + 1) -
                             getVoxel(vd, idx, idy, idz - 1))/
                        vd->sliceDist[2] * 0.5f;
                }
#endif
                gp += 3;
            }
        }
    }
    return gradients;
}


void filterGradients(VolumeData *vd, float *gradients)
{
    const int fSize = GRAD_FILTER_SIZE;
    int fw = GRAD_FILTER_SIZE/2; // filter width
    int gi, ogi;
    int borderDist[3];
    float sum;
    float *filteredGrad = NULL;
    float *filter = NULL; 

    fprintf(stderr, "Filtering gradients ... may also take a while\n");

    filteredGrad = new float[3*vd->size[0]*vd->size[1]*vd->size[2]];
    // allocate memory for filter kernels
    filter = new float[fSize*fSize*fSize];

    // Compute the filter kernels
    sum = 0.0f;
    for (int k=-fw; k < fw-1; ++k)
    {
        for (int j=-fw; j < fw-1; ++j)
        {
            for (int i=-fw; i < fw-1; ++i) 
            {
                sum += filter[((fw+k)*fSize + fw+j)*fSize + fw+i]
                    = exp(-(float)(SQR(i) + SQR(j) + SQR(k)) / SIGMA2);
            }
        }
    }
    for (int k=-fw; k < fw-1; ++k)
        for (int j=-fw; j < fw-1; ++j)
            for (int i=-fw; i < fw-1; ++i) 
                filter[((fw+k)*fSize + fw+j)*fSize + fw+i] /= sum;

    gi = 0;
    // Filter the gradients 
    for (int z=0; z < vd->size[2]; ++z) 
    {
        for (int y=0; y < vd->size[1]; ++y) 
        {
            for (int x=0; x < vd->size[0]; ++x) 
            {
                borderDist[0] = MIN(x, vd->size[0] - x - 1);
                borderDist[1] = MIN(y, vd->size[1] - y - 1);
                borderDist[2] = MIN(z, vd->size[2] - z - 1);

                fw = MIN(GRAD_FILTER_SIZE/2,
                         MIN(MIN(borderDist[0], 
                                 borderDist[1]),
                             borderDist[2]));

                // for each dimension 
                for (int n=0; n < 3; ++n)
                {
                    filteredGrad[gi] = 0.0f;
                    for (int k=-fw; k < fw-1; ++k)
                    {
                        for (int j=-fw; j < fw-1; ++j) 
                        {
                            for (int i=-fw; i < fw-1; ++i) 
                            {
                                ogi = 3*(((z + k)*vd->size[1] 
                                          + (y + j))*vd->size[0] + (x + i)) + n;
                                filteredGrad[gi] 
                                    += filter[((fw+k)*fSize + fw+j)*fSize + fw+i] * gradients[ogi];
                            }
                        }
                    }
                    ++gi;
                }
            }
        }
    }

    // Replace the orignal gradients by the filtered gradients 
    memcpy(gradients, filteredGrad, 
           3*vd->size[0] * vd->size[1] * vd->size[2] * sizeof(float));

    delete [] filteredGrad;

    // free memory used for filter
    delete [] filter;
}


void quantize8(float *grad, unsigned char *data)
{
    float len;

    len = sqrt(SQR(grad[0]) + SQR(grad[1]) + SQR(grad[2]));

    if (len < EPS) {
        grad[0] = grad[1] = grad[2] = 0.0f;
    } else {
        grad[0] /= len;
        grad[1] /= len;
        grad[2] /= len;
    }

    for (int i=0; i<3; ++i)
        data[i] = (unsigned char)((grad[i] + 1.0)/2.0 * UCHAR_MAX);
}


void quantize16(float *grad, unsigned short *data)
{
    float len;

    len = sqrt(SQR(grad[0]) + SQR(grad[1]) + SQR(grad[2]));

    if (len < EPS) {
        grad[0] = grad[1] = grad[2] = 0.0f;
    } else {
        grad[0] /= len;
        grad[1] /= len;
        grad[2] /= len;
    }

    for (int i=0; i<3; ++i)
        data[i] = (unsigned short)((grad[i] + 1.0)/2.0 * USHRT_MAX);
}


void* quantizeGradients(VolumeData *vd, float *gradIn,
                        DataType dataTypeOut)
{
    int di = 0;
    int size = vd->size[0]*vd->size[1]*vd->size[2];
    void *gradOut = NULL;

    switch (dataTypeOut) 
    {
    case DATRAW_UCHAR:
        gradOut = new unsigned char[3*size];
        for (int i=0; i<size; ++i)
        {
            quantize8(&gradIn[di], &((unsigned char*)gradOut)[di]);
            di += 3;
        }
        break;
    case DATRAW_USHORT:
        gradOut = new unsigned short[3*size];
        for (int i=0; i<size; ++i)
        {
            quantize16(&gradIn[di], &((unsigned short*)gradOut)[di]);
            di += 3;
        }
        break;
    default:
        fprintf(stderr, "Gradients:  Unsupported data type for quantization.\n");
        return NULL;
        break;
    }

    return gradOut;
}
