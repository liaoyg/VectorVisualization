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
 * $Id: trackball.cpp,v 1.2 2008/05/09 08:23:22 falkmn Exp $ 
 */

#define _USE_MATH_DEFINES
#include <math.h>
#include "mmath.h"
#include "trackball.h"

float projectToSphere(float radius, float x, float y)
{
  float dist, t, z;

  dist = sqrt(x * x + y * y);
  if (dist < radius * M_SQRT1_2) 
  { /* Inside sphere. */
    z = sqrt(radius * radius - dist * dist);
  } 
  else
  { /* On hyperbola. */
    t = radius / (float)M_SQRT2;
    z = t * t / dist;
  }
  return z;
}


Quaternion trackBall(float pX, float pY, float qX, float qY)
{
  Vector3 v1, v2, t;
  Vector3 cross;
  float rot;
  float phi;

  if ((fabs(pX - qX) < EPS) && (fabs(pY - qY) < EPS))
  {
    return Quaternion_new(0.0, 0.0, 0.0, 1.0);
  }

  // First, figure out z-coordinates for projection of P1 and
  // P2 to deformed sphere.
  v1 = Vector3_new(pX, pY, projectToSphere(TRACKBALLSIZE, pX, pY));
  v2 = Vector3_new(qX, qY, projectToSphere(TRACKBALLSIZE, qX, qY));

  cross = Vector3_cross(v1, v2);

  // figure out how much to rotate
  t = Vector3_sub(v1, v2);
  rot = sqrt(t.x*t.x + t.y*t.y + t.z*t.z) / (2.0f * TRACKBALLSIZE);

  rot = (rot > 1.0f) ? 1.0f : ((rot < -1.0f) ? -1.0f : rot);
  phi = 2.0f * asin(rot);

  return Quaternion_fromAngleAxis(phi, cross);
}

void buildRotmatrix(float m[4][4], Quaternion q)
{
  m[0][0] = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
  m[0][1] = 2.0f * (q.x * q.y + q.z * q.w);
  m[0][2] = 2.0f * (q.z * q.x - q.y * q.w);
  m[0][3] = 0.0f;

  m[1][0] = 2.0f * (q.x * q.y - q.z * q.w);
  m[1][1] = 1.0f - 2.0f * (q.z * q.z + q.x * q.x);
  m[1][2] = 2.0f * (q.y * q.z + q.x * q.w);
  m[1][3] = 0.0f;

  m[2][0] = 2.0f * (q.z * q.x + q.y * q.w);
  m[2][1] = 2.0f * (q.y * q.z - q.x * q.w);
  m[2][2] = 1.0f - 2.0f * (q.y * q.y + q.x * q.x);
  m[2][3] = 0.0f;

  m[3][0] = 0.0f;
  m[3][1] = 0.0f;
  m[3][2] = 0.0f;
  m[3][3] = 1.0f;
}
