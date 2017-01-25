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
 * Filename: trackball.cpp
 * 
 * $Id: trackball.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */

#ifndef _TRACKBALL_H_
#define _TRACKBALL_H_

#include "mmath.h"

#define TRACKBALLSIZE  0.8f

/* Project an x,y pair onto a sphere of radius r OR a
   hyperbolic sheet if we are away from the center of the
   sphere. */
float projectToSphere(float radius, float x, float y);
Quaternion trackBall(float pX, float pY, float qX, float qY);

void buildRotmatrix(float m[4][4], Quaternion q);


#endif // _TRACKBALL_H_
