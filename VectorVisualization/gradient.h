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
 * Filename: gradient.h
 * 
 * $Id: gradient.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _GRADIENT_H_
#define _GRADIENT_H_

//#include "tools/reader.h"
#include "dataSet.h"
#include "types.h"


#define SOBEL                1
#define GRAD_FILTER_SIZE     5
#define SIGMA2             5.0f



typedef struct GradientData
{
  void *volData;
  int numSlices[3];
  float sliceDists[3];
} GradientData;


// load precomputed gradients from a file
// the extension of the gradient file (.grd or .grdf) is appended to fileName
void* loadGradients(VolumeData *vd, char *fileName, 
                    DataType dataType, bool useFloat=false);
// save computed gradients to a file
// the extension of the gradient file (.grd or .grdf) is appended to fileName
bool saveGradients(VolumeData *vd, char *fileName, 
                   void *gradients, DataType dataType, 
                   bool useFloat=false);

// allocate and compute gradients 
float* computeGradients(VolumeData *vd);

// filter gradients
void filterGradients(VolumeData *vd, float *gradients);

// quantize gradients to unsigned char or unsigned short
void* quantizeGradients(VolumeData *vd, float *gradIn, 
                        DataType dataTypeOut);

#endif // _GRADIENT_H_
