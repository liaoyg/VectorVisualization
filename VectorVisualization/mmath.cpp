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
 * Filename: mmath.cpp
 * 
 * $Id: mmath.cpp,v 1.2 2008/05/09 08:23:24 falkmn Exp $ 
 */

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include "mmath.h"



int nextPowerTwo(int width)
{
  int i = 1;
  while (i < width) i <<= 1;
  return i;
}


Vector3 Vector3_new(float x, float y, float z)
{
  Vector3 result;
	
  result.x = x;
  result.y = y;
  result.z = z;
	
  return result;
}


Vector3 Vector3_sign(Vector3 u)
{
  Vector3 result;

  result.x = (u.x < 0.0) ? -1.0f : 1.0f;
  result.y = (u.y < 0.0) ? -1.0f : 1.0f;
  result.z = (u.z < 0.0) ? -1.0f : 1.0f;

  return result;
}


void Vector3_abs(Vector3 *u)
{
  u->x = fabs(u->x);
  u->y = fabs(u->y);
  u->z = fabs(u->z);
}


Vector3 Vector3_add(Vector3 u, Vector3 v)
{
  Vector3 result;

  result.x = u.x + v.x;
  result.y = u.y + v.y;
  result.z = u.z + v.z;

  return result;
}	


Vector3 Vector3_sub(Vector3 u, Vector3 v)
{
  Vector3 result;
	
  result.x = u.x - v.x;
  result.y = u.y - v.y;
  result.z = u.z - v.z;

  return result;
}	


Vector3 Vector3_smult(float s, Vector3 v)
{
  Vector3 result;
	
  result.x = s * v.x;
  result.y = s * v.y;
  result.z = s * v.z;
	
  return result;	
}


Vector3 Vector3_mult(Vector3 u, Vector3 v)
{
  Vector3 result;
	
  result.x = u.x * v.x;
  result.y = u.y * v.y;
  result.z = u.z * v.z;
	
  return result;	
}


Vector3 Vector3_cross(Vector3 u, Vector3 v) 
{
  Vector3 result;

  result.x = u.y * v.z - u.z * v.y;
  result.y = u.z * v.x - u.x * v.z;
  result.z = u.x * v.y - u.y * v.x;

  return result;
}


float Vector3_dot(Vector3 u, Vector3 v) 
{
  return u.x*v.x + u.y*v.y + u.z*v.z;
}


float Vector3_normalize(Vector3 *v)
{
  float d = (float)sqrt(SQR(v->x) + SQR(v->y) + SQR(v->z));

  if (d > EPS) {
    v->x /= d;
    v->y /= d;
    v->z /= d;
    return d;
  } else {
    v->x = v->y = v->z = 0.f;
    return 0.f;
  }
}


void Vector3_stderr(char *s, Vector3 v)
{
  fprintf(stderr, "%s <%f, %f, %f>\n", s, v.x, v.y, v.z);
}


Quaternion Quaternion_new(float x, float y, float z, float w)
{
  Quaternion result;

  result.x = x;
  result.y = y;
  result.z = z;
  result.w = w;
	
  return result;
}


Quaternion Quaternion_fromAngleAxis(float angle, Vector3 axis)
{
  Quaternion q;
  float l = Vector3_normalize(&axis);

  if(l > EPS){
    l = (float)sin(0.5f * angle);
    q.x = axis.x * l;
    q.y = axis.y * l;
    q.z = axis.z * l;
    q.w = (float)cos(0.5f * angle);
  } else {
    q.x = 0.f;
    q.y = 0.f;
    q.z = 0.f;
    q.w = 1.f;
  }

  return q;
} 


Quaternion Quaternion_mult(Quaternion p, Quaternion q)
{
  Quaternion result;

  result.w = p.w * q.w - (p.x * q.x + p.y * q.y + p.z * q.z);
  result.x = p.w * q.x + q.w * p.x + p.y * q.z - p.z * q.y;
  result.y = p.w * q.y + q.w * p.y + p.z * q.x - p.x * q.z;
  result.z = p.w * q.z + q.w * p.z + p.x * q.y - p.y * q.x;

  return result;
}


void Quaternion_getAngleAxis(const Quaternion q, float *angle, Vector3 *axis)
{
  double d = sqrt(SQR((double)q.x) + SQR((double)q.y) + SQR((double)q.z));
 
  if(d > 1e-6){
    d = 1.f / d;
    axis->x = (float)(q.x * d);
    axis->y = (float)(q.y * d);
    axis->z = (float)(q.z * d);
    if (1.0 - (fabs(q.w)) > 1e-6)
      *angle   = 2.f * (float)acos(q.w);
    else
      *angle   = 0.0;
  } else {
    axis->x = 0.f;
    axis->y = 0.f;
    axis->z = 1.f;
    *angle  = 0.f;
  }
} 


void Quaternion_normalize(Quaternion *q)
{
  float d = (float)sqrt(SQR(q->w) + SQR(q->x) + SQR(q->y) + SQR(q->z));
  if (d > EPS) {
    d = 1.f / d;
    q->w *= d;
    q->x *= d;
    q->y *= d;
    q->z *= d;
  } else {
    q->w = 1.f;
    q->x = q->y = q->z = 0.f;
  }
}


Quaternion Quaternion_inverse(Quaternion q)
{
  Quaternion result;
  float d = SQR(q.w) + SQR(q.x) + SQR(q.y) + SQR(q.z);
  if (d > EPS) {
    d = 1.f / (float)sqrt(d);
    result.w = q.w * d;
    result.x = -q.x * d;
    result.y = -q.y * d;
    result.z = -q.z * d;
  } else {
    result.w = 1.f;
    result.x = result.y = result.z = 0.f;
  }
  return result;
}


Vector3 Quaternion_multVector3(Quaternion q, Vector3 v)
{
  Vector3 result, u;
  float uu, uv;

  u.x = q.x;
  u.y = q.y;
  u.z = q.z;	

  uu = Vector3_dot(u, u);
  uv = Vector3_dot(u,v);

  result = Vector3_smult(2.f, Vector3_add(Vector3_smult(uv, u), 
                                          Vector3_smult(q.w, Vector3_cross(u, v))));
  result = Vector3_add(result, Vector3_smult(SQR(q.w) - uu, v));

  return result;
}


void Quaternion_stderr(char *s, Quaternion q)
{
  fprintf(stderr, "%s (%f <%f, %f, %f>)\n", s, q.w, q.x, q.y, q.z);
}
