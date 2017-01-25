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
 * Filename: camera.cpp
 * 
 * $Id: camera.cpp,v 1.2 2008/05/09 08:23:22 falkmn Exp $ 
 */

#ifdef _WIN32
#  include <windows.h>
#endif

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <stdio.h>

#include "transform.h"
#include "mmath.h"
#include "camera.h"


Camera::Camera(void) : _nearClip(0.1f),_farClip(50.0f),_fovy(35.0f),
                       _useHaltonSequence(false),_camSaved(false),
                       _sequence(NULL),_seqIdx(0),_seqLength(0)
{
    _dist = 4.0f;
}


Camera::~Camera(void)
{
    delete [] _sequence;
}


void Camera::setCamera(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(_fovy, _aspect, _nearClip, _farClip);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(_pos.x, _pos.y, _pos.z);
    glTranslatef(0.0, 0.0, -_dist);
    glRotatef(static_cast<float>(_angle * 180.0/M_PI), _axis.x, _axis.y, _axis.z);
}


void Camera::translate(int xNew, int yNew, int xOld, int yOld)
{
    _pos.x += MOUSE_SCALE * (xNew - xOld) / (float)_w;
    _pos.y += MOUSE_SCALE * (yOld - yNew) / (float)_h;
}


void Camera::dolly(int xNew, int yNew, int xOld, int yOld)
{
    _pos.z -= 2.0f*MOUSE_SCALE * (yOld - yNew) / (float)_h;
}


void Camera::enableHaltonPos(bool enable)
{
    _useHaltonSequence = enable;

    if (enable)
    {
        // store current camera position
        // and update to first halton position
        _savedCam_internal = _q_internal;
        _savedCam_dist = _dist;


    }
    else
    {
        // restore camera position
        _q_internal = _savedCam_internal;
        _dist = _savedCam_dist;
    }

    update();
}


bool Camera::loadHaltonPositions(const char *fileName)
{
    FILE *fp;

    if (_sequence)
    {
        delete [] _sequence;
        _sequence = NULL;
    }

    if (!fileName)
    {
        fprintf(stderr, "HaltonPosition:   No filename given.\n");
        return false;
    }

    fp = fopen(fileName, "rb");
    if (!fp)
    {
        fprintf(stderr, "HaltonPosition:   Could not open halton sequence "
            "\"%s\".\n", fileName);
        return false;
    }

    if (fread(&_seqLength, sizeof(int), 1, fp) != 1)
    {
        fprintf(stderr, "HaltonPosition:   Could not parse halton sequence "
            "in \"%s\".\n", fileName);
        return false;
    }

    _sequence = new float[4*_seqLength];

    if (fread(_sequence, 4*_seqLength*sizeof(float), 1, fp) != 1)
    {
        fprintf(stderr, "HaltonPosition:   Could not load halton sequence "
            "from \"%s\".\n", fileName);
        fclose(fp);
        delete [] _sequence;
        _sequence = NULL;
        return false;
    }

    fclose(fp);

    fprintf(stdout, "HaltonPosition:   Sequence file \"%s\"\n", fileName);
    return true;
}


void Camera::nextHaltonPos(void)
{
    Quaternion q;
    Vector3 p;
    Vector3 axis;
    Vector3 e_3;
    float phi;

    if (!_sequence)
    {
        _q_internal.x = 0.0f;
        _q_internal.y = 0.0f;
        _q_internal.z = 0.0f;
        _q_internal.w = 1.0f;

        _dist = 2.0f;

        return;
    }

    if (_seqIdx > _seqLength - 1)
    {
        fprintf(stderr, "HaltonPosition:   Halton sequence contains too "
            "few elements. Resetting.\n");
        _seqIdx = 0;
    }

    // get position from halton sequence
    p.x = _sequence[4*_seqIdx];
    p.y = _sequence[4*_seqIdx+1];
    p.z = _sequence[4*_seqIdx+2];
    // find quaternion for translating (0,0,1) into p
    if ((fabs(p.x) > EPS) || (fabs(p.y) > EPS) || (p.z < -EPS))
    {
        e_3 = Vector3_new(0.0f, 0.0f, 1.0f);

        phi = acos(Vector3_dot(e_3, p));
        axis = Vector3_cross(e_3, p);

        q = Quaternion_fromAngleAxis(phi, axis);
        _q_internal.x = q.x;
        _q_internal.y = q.y;
        _q_internal.z = q.z;
        _q_internal.w = q.w;
    }
    else // no rotation needed as p lies already in (0,0,1)
    {
        _q_internal.x = 0.0f;
        _q_internal.y = 0.0f;
        _q_internal.z = 0.0f;
        _q_internal.w = 1.0f;
    }

    _dist = 4.0;

    // increase sequence index for next time
    ++_seqIdx;
}

