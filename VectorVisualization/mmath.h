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
 * Filename: mmath.h
 * 
 * $Id: mmath.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _MMATH_H_
#define _MMATH_H_

#define _USE_MATH_DEFINES

#include <math.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif


#ifndef EPS
#  define EPS 1e-5f
#endif

#ifndef SQR
#  define SQR(a) ((a) * (a))
#endif
#ifndef MAX
#  define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#  define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef M_PI
#  define M_PI      3.14159265358979323846
#endif
#ifndef M_SQRT2
#  define M_SQRT2   1.41421356237309504880
#endif
#ifndef M_SQRT1_2
#  define M_SQRT1_2 0.70710678118654752440
#endif


typedef struct Vector3 {
	float x, y, z;
} Vector3;

typedef struct Quaternion {
	float x, y, z, w;
} Quaternion;


int nextPowerTwo(int width);


Vector3 Vector3_new(float x, float y, float z);

Vector3 Vector3_sign(Vector3 u);

void Vector3_abs(Vector3 *u);

Vector3 Vector3_add(Vector3 u, Vector3 v);

Vector3 Vector3_sub(Vector3 u, Vector3 v);

Vector3 Vector3_cross(Vector3 u, Vector3 v);

float Vector3_dot(Vector3 u, Vector3 v);

Vector3 Vector3_mult(Vector3 u, Vector3 v);

Vector3 Vector3_smult(float s, Vector3 v);

float Vector3_normalize(Vector3 *v);

void Vector3_stderr(char *s, Vector3 v);


Quaternion Quaternion_new(float x, float y, float z, float w);

Quaternion Quaternion_fromAngleAxis(float angle, Vector3 axis);

Quaternion Quaternion_mult(Quaternion p, Quaternion q);

void Quaternion_getAngleAxis(const Quaternion q, float *angle, Vector3 *axis);

void Quaternion_normalize(Quaternion *q);

Quaternion Quaternion_inverse(Quaternion q);

Vector3 Quaternion_multVector3(Quaternion q, Vector3 v);


void Quaternion_stderr(char *s, Quaternion q);

#ifdef __cplusplus
}
#endif


#endif // _MMATH_H_
