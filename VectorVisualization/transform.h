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
 * Filename: transform.cpp
 * 
 * $Id: transform.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#ifdef _WIN32
#include <windows.h>
#endif

#define _USE_MATH_DEFINES
//#include <GL/gl.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include "slicing.h"
#include "mmath.h"

#define LOCK_VARIANCE   15.0


class Transform
{
public:
    Transform(void);
    ~Transform(void);

    void setQuaternion(Quaternion &q);
    Quaternion getQuaternion(void) { return _q; }

    Vector3 getPosition(void);

    // perform a rotation with Quaternion q_rot
    void rotate(Quaternion q_rot);
    // perform a rotation with Quaternion q_rot in camera space
    void rotate(Quaternion q_rot, Quaternion q_cam);

    // rotate using a trackball
    void rotate(int xNew, int yNew, int xOld, int yOld);
    // rotate using a trackball in camera space
    void rotate(int xNew, int yNew, int xOld, int yOld, Quaternion q_cam);


    // if enable == false and the position was locked,
    // _q_internal will be copied to _q
    void setLock(bool enable);
    bool isLocked(void) { return _locked; }

    float getAngle(void) { return _angle; }
    Vector3 getAxis(void) { return _axis; }
    void getAxis(float *axis);

    void setDistance(float d) { _dist = d; }
    float getDistance(void) { return _dist; }

    void setWindowWidth(const int width) { _w = width; _aspect = (float)_w/_h; }
    const int getWindowWidth(void) { return _w; }
    void setWindowHeight(const int height) { _h = height; _aspect = (float)_w/_h; }
    const int getWindowHeight(void) { return _h; }
    void setWindow(const int width, const int height)
    { _w = width; _h = height; _aspect = (float)_w/_h; }

    const float getAspectRatio(void) { return _aspect; }

    float operator+=(float d) { _dist += d; return _dist; }
    float operator-=(float d) { _dist -= d; return _dist; }
    float operator*=(float d) { _dist *= d; return _dist; }
    float operator/=(float d) { _dist /= d; return _dist; }

protected:
    // update _q according to _q_internal and lock state
    void update(void);

    // cosine of LOCK_VARIANCE
    static const float cos_var;
    // inverse square root of 3
    static const float sqrt1_3;

    // external representation
    // when lock is enabled, this quaternion snaps to the corners
    // and face centers of a cube
    Quaternion _q;
    // internal representation
    Quaternion _q_internal;
    bool _locked;

    // quaternion rotation decomposed in angle and axis
    float _angle;
    Vector3 _axis;

    // distance to origin
    float _dist;

    // window width and height
    int _w;
    int _h;
    float _aspect;

private:
};


class ClipPlane : public Transform
{
public:
    ClipPlane(void);
    ~ClipPlane(void);

    // set OpenGL id of clip plane
    // e.g. GL_CLIP_PLANE0
    void setPlaneId(GLenum planeId) { _planeId = planeId; }

    // set the size of the volume to be clipped
    void setBoundingBox(Vector3 &bbMin, Vector3 &bbMax);
    void setBoundingBox(float xmin, float ymin, float zmin,
                        float xmax, float ymax, float zmax);

    void setColor(float r, float g, float b, float a);
    // sets the normal of the clip plane, the forth component
    // is the distance to the origin
    void setNormal(double x, double y, double z, double d);

    // returns the normal of the clip plane, the forth component
    // is the distance to the origin
    double* getNormal(void) { return _normal; }
    // set only the forth component of the normal
    void setDistance(float d);

    
    using Transform::rotate;
    void rotate(Quaternion q);
    void rotate(int xNew, int yNew, int xOld, int yOld, Quaternion q_cam);
    // perform a rotation with Quaternion q_rot in camera space
    void rotate(Quaternion q_rot, Quaternion q_cam);

    // show proxy geometry of the clip plane
    void draw(void);

    // fill only the hole of the clipped volume
    void drawSlice(void);

    // enable the OpenGL clipstate
    // planeId should be GL_CLIP_PLANE[n]
    void enable(GLenum planeId);
    inline void enable(void)
    {
        if (_active)
        {
            glEnable(_planeId);
            glClipPlane(_planeId, _normal);
        }
    }

    inline void disable(void) { glDisable(_planeId); }

    // the clip plane only clips when active
    void setActive(bool active) { _active = active; }
    bool isActive(void) { return _active; }

    void setVisible(bool visible) { _visible = visible; }
    bool isVisible(void) { return _visible; }

    friend void switchClipPlane(ClipPlane **currentClipPlane,
                                ClipPlane *newClipPlane);

    double operator+=(double d) { _normal[3] += d; return _normal[3]; }
    double operator-=(double d) { _normal[3] -= d; return _normal[3]; }
    double operator*=(double d) { _normal[3] *= d; return _normal[3]; }
    double operator/=(double d) { _normal[3] /= d; return _normal[3]; }

protected:
private:
    // state of clip plane
    bool _active;
    // visibility of the clip plane
    bool _visible;
    float _color[4];
    GLenum _planeId;

    // normal of the clip plane
    // double precision for OpenGL command
    double _normal[4];
    // initial (unrotated) normal 
    double _initialNormal[4];
    // bounding box of volume to be clipped
    Vector3 _bbMin;
    Vector3 _bbMax;

    float _extent[3];

    ViewSlicing _slice;
};


// if currentCp == newCp the active flag is toggled
void switchClipPlane(ClipPlane **currentClipPlane,
                     ClipPlane *newClipPlane);

#endif // _TRANSFORM_H_
