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
 * $Id: transform.cpp,v 1.3 2008/05/13 08:55:30 falkmn Exp $ 
 */

#ifdef _WIN32
#  include <windows.h>
#endif

//#include <GL/gl.h>
#include "trackball.h"
#include "transform.h"


const float Transform::cos_var = (float)cos(LOCK_VARIANCE * M_PI/180.0);
const float Transform::sqrt1_3 = 1.0f/sqrt(3.0f);


Transform::Transform(void) : _locked(false),_dist(0.0f),_w(1),_h(1),
                             _aspect(1.0f)
{
    _q.x = _q.y = _q.z = 0.0f;
    _q.w = 1.0f;
    _q_internal.x = _q_internal.y = _q_internal.z = 0.0f;
    _q_internal.w = 1.0f;

    Quaternion_getAngleAxis(_q, &_angle, &_axis);
}


Transform::~Transform(void)
{
}


void Transform::setQuaternion(Quaternion &q)
{
    _q_internal.x = q.x;
    _q_internal.x = q.y;
    _q_internal.x = q.z;
    _q_internal.x = q.w;

    update();
}


Vector3 Transform::getPosition(void)
{
    Vector3 pos;
    
    pos.x = 0.0;
    pos.y = 0.0;
    pos.z = _dist;
    return Quaternion_multVector3(_q, pos);
}


// perform a rotation with Quaternion q_rot
void Transform::rotate(Quaternion q_rot)
{
    _q_internal = Quaternion_mult(q_rot, _q_internal);
    Quaternion_normalize(&_q_internal);

    update();

    Quaternion_getAngleAxis(_q, &_angle, &_axis);
}


// perform a rotation with Quaternion q_rot in camera space
void Transform::rotate(Quaternion q_rot, Quaternion q_cam)
{
    // TODO: proove rotation!!!
    Quaternion cam_conj, r;

    cam_conj.x = -q_cam.x;
    cam_conj.y = -q_cam.y;
    cam_conj.z = -q_cam.z;
    cam_conj.w =  q_cam.w;

    r = Quaternion_mult(q_cam, _q_internal);
    _q_internal = Quaternion_mult(cam_conj, Quaternion_mult(q_rot, r));
    Quaternion_normalize(&_q_internal);

    update();

    Quaternion_getAngleAxis(_q, &_angle, &_axis);
}


void Transform::rotate(int xNew, int yNew, int xOld, int yOld)
{
    Quaternion q = trackBall((2.0f*xOld - _w) / _w,
                             (_h - 2.0f*yOld) / _h,
                             (2.0f*xNew - _w) / _w,
                             (_h - 2.0f*yNew) / _h);
    rotate(q);
}


void Transform::rotate(int xNew, int yNew, int xOld, int yOld, Quaternion q_cam)
{
    Quaternion q = trackBall((2.0f*xOld - _w) / _w,
                             (_h - 2.0f*yOld) / _h,
                             (2.0f*xNew - _w) / _w,
                             (_h - 2.0f*yNew) / _h);
    rotate(q, q_cam);
}


// if enable == false and the position was locked,
// _q_internal will be copied to _q
void Transform::setLock(bool enable)
{
    if (!enable && _locked)
    {
        // use locked position as new origin
        _q_internal.x = _q.x;
        _q_internal.y = _q.y;
        _q_internal.z = _q.z;
        _q_internal.w = _q.w;
    }

    _locked = enable;
}


void Transform::getAxis(float *axis)
{
    if (!axis) 
        return; 
    axis[0] = _axis.x;
    axis[1] = _axis.y;
    axis[2] = _axis.z;
}


// update _q according to _q_internal and lock state
void Transform::update(void)
{
    Quaternion p, q_conj;
    Vector3 axis;
    Vector3 sign;
    Vector3 v_vec, v_lock;
    float phi;

    q_conj.x = -_q_internal.x;
    q_conj.y = -_q_internal.y;
    q_conj.z = -_q_internal.z;
    q_conj.w =  _q_internal.w;

    // check if locked position is wanted
    if (_locked)
    {
        p.x =  0.0f;
        p.y =  0.0f;
        p.z = -1.0f;
        p.w =  0.0f;
        p = Quaternion_mult(Quaternion_mult(_q_internal, p), q_conj);

        v_vec = Vector3_new(p.x, p.y, p.z);
        v_lock = v_vec;
        sign = Vector3_sign(v_lock);
        Vector3_abs(&v_lock);

        if (v_lock.x > cos_var)     // x axis
        {
            v_lock.x = 1.0f;
            v_lock.y = v_lock.z = 0.0f;
        }
        else if (v_lock.y > cos_var)     // y axis
        {
            v_lock.y = 1.0f;
            v_lock.x = v_lock.z = 0.0f;
        }
        else if (v_lock.z > cos_var)     // z axis
        {
            v_lock.z = 1.0f;
            v_lock.x = v_lock.y = 0.0f;
        }
        else if ((v_lock.x + v_lock.y)*M_SQRT1_2 > cos_var)  // <1,1,0> plane
        {
            v_lock.x = v_lock.y = (float)M_SQRT1_2;
            v_lock.z = 0.0f;
        }
        else if ((v_lock.y + v_lock.z)*M_SQRT1_2 > cos_var)  // <0,1,1> plane
        {
            v_lock.y = v_lock.z = (float)M_SQRT1_2;
            v_lock.x = 0.0f;
        }
        else if ((v_lock.x + v_lock.z)*M_SQRT1_2 > cos_var)  // <1,0,1> plane
        {
            v_lock.x = v_lock.z = (float)M_SQRT1_2;
            v_lock.y = 0.0f;
        }
        else if ((v_lock.x+v_lock.y+v_lock.z)*sqrt1_3 > cos_var) // <1,1,1> plane
        {
            v_lock.x = v_lock.y = v_lock.z = sqrt1_3;
        }

        v_lock = Vector3_mult(v_lock, sign);
        axis = Vector3_cross(v_vec, v_lock);
        phi = acos(Vector3_dot(v_vec, v_lock));
        // build new quaternion with locked coordinates
        _q = Quaternion_mult(Quaternion_fromAngleAxis(phi, axis), 
                             _q_internal);
    }
    else
    {
        // non-locking, copy internal quaternion
        _q.x = _q_internal.x;
        _q.y = _q_internal.y;
        _q.z = _q_internal.z;
        _q.w = _q_internal.w;
    }
}


// --------------------------------------


ClipPlane::ClipPlane(void) : _active(false),_visible(false),
                             _planeId(GL_CLIP_PLANE0)
{
    _color[0] = 0.0f;
    _color[1] = 0.0f;
    _color[2] = 1.0f;
    _color[3] = 0.05f;

    _normal[0] = _initialNormal[0] =  0.0f;
    _normal[1] = _initialNormal[1] =  0.0f;
    _normal[2] = _initialNormal[2] = -1.0f;
    _normal[3] = _initialNormal[3] =  0.0f;
}


ClipPlane::~ClipPlane(void)
{
}


void ClipPlane::setBoundingBox(Vector3 &bbMin, Vector3 &bbMax)
{
    _bbMax.x = MAX(bbMax.x, MAX(bbMax.y, bbMax.z));
    _bbMin.x = MIN(bbMin.x, MIN(bbMin.y, bbMin.z));
    _bbMax.y = _bbMax.z = _bbMax.x;
    _bbMin.y = _bbMin.z = _bbMin.x;

    _extent[0] = bbMax.x - bbMin.x;
    _extent[1] = bbMax.y - bbMin.y;
    _extent[2] = bbMax.z - bbMin.z;
}


void ClipPlane::setBoundingBox(float xmin, float ymin, float zmin,
                               float xmax, float ymax, float zmax)
{
    _bbMax.x = MAX(xmax, MAX(ymax, zmax));
    _bbMin.x = MIN(xmin, MIN(ymin, zmin));
    _bbMax.y = _bbMax.z = _bbMax.x;
    _bbMin.y = _bbMin.z = _bbMin.x;

    _extent[0] = xmax - xmin;
    _extent[1] = ymax - ymin;
    _extent[2] = zmax - zmin;
}


void ClipPlane::setColor(float r, float g, float b, float a)
{
    _color[0] = r; _color[1] = g; 
    _color[2] = b; _color[3] = a;
}


void ClipPlane::setNormal(double x, double y, double z, double d)
{
    Vector3 v = Vector3_new((float)x, (float)y, (float)z);

    // update normal
    _normal[0] = x;
    _normal[1] = y;
    _normal[2] = z;
    _normal[3] = d;

    _axis = Vector3_cross(v, Vector3_new(0.0f, 0.0f, -1.0f));
    _angle = Vector3_dot(v,  Vector3_new(0.0f, 0.0f, -1.0f));

    _q_internal = Quaternion_fromAngleAxis(_angle, _axis);

    update();

    Quaternion_getAngleAxis(_q, &_angle, &_axis);

    glClipPlane(_planeId, _normal);
}


void ClipPlane::setDistance(float d)
{
    _normal[3] = _initialNormal[3] = d;

    glClipPlane(_planeId, _normal);
}


void ClipPlane::rotate(Quaternion q)
{
    Vector3 v;

    Transform::rotate(q);


    // update normal

    // apply rotation to initial normal
    v = Vector3_new((float)_initialNormal[0],
                    (float)_initialNormal[1], 
                    (float)_initialNormal[2]);
    v = Quaternion_multVector3(_q, v);

    _normal[0] = v.x;
    _normal[1] = v.y;
    _normal[2] = v.z;

    glClipPlane(_planeId, _normal);
}


void ClipPlane::rotate(int xNew, int yNew, int xOld, int yOld, Quaternion q_cam)
{
    Quaternion q = trackBall((2.0f*xOld - _w) / _w,
                             (_h - 2.0f*yOld) / _h,
                             (2.0f*xNew - _w) / _w,
                             (_h - 2.0f*yNew) / _h);
    rotate(q, q_cam);
}

void ClipPlane::rotate(Quaternion q_rot, Quaternion q_cam)
{
//    Quaternion q_conj, p;
    Vector3 v;

    Transform::rotate(q_rot, q_cam);

    // update normal

    // apply rotation to initial normal
    v = Vector3_new((float)_initialNormal[0], 
                    (float)_initialNormal[1], 
                    (float)_initialNormal[2]);
    v = Quaternion_multVector3(_q, v);

    _normal[0] = v.x;
    _normal[1] = v.y;
    _normal[2] = v.z;

    glClipPlane(_planeId, _normal);
}


void ClipPlane::draw(void)
{
    float scale = 1.4f;
    float lineScale = 1.04f * scale;

    if (!_visible)
        return;

    // save state of blending and culling
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT);

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glPushMatrix();

    glRotatef(_angle*180.0f/(float)M_PI, _axis.x, _axis.y, _axis.z);

    glTranslated(0.0, 0.0, _normal[3]);

    glColor4fv(_color);
    glBegin(GL_QUADS);
    {
        glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(scale*_bbMax.x, scale*_bbMin.y, 0.0);
        glVertex3f(scale*_bbMax.x, scale*_bbMax.y, 0.0);
        glVertex3f(scale*_bbMin.x, scale*_bbMax.y, 0.0);
        glVertex3f(scale*_bbMin.x, scale*_bbMin.y, 0.0);
    }
    glEnd();

    glColor4f(1.0f, 1.0f, 0.0f, 0.8f);
    glBegin(GL_LINE_LOOP);
    {
        glVertex3f(lineScale*_bbMax.x, lineScale*_bbMin.y, 0.0);
        glVertex3f(lineScale*_bbMax.x, lineScale*_bbMax.y, 0.0);
        glVertex3f(lineScale*_bbMin.x, lineScale*_bbMax.y, 0.0);
        glVertex3f(lineScale*_bbMin.x, lineScale*_bbMin.y, 0.0);
    }
    glEnd();

    glPopMatrix();
    glDepthMask(GL_TRUE);
    glPopAttrib();
}


void ClipPlane::drawSlice(void)
{
    if (!_active)
        return;

    _slice.setupSingleSlice(_normal, _extent);
    
    //glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLE_FAN);
    {
        _slice.drawSingleSlice((float)-(_normal[3] - 0.0001));
    }
    glEnd();
    //glEnable(GL_CULL_FACE);
}


void ClipPlane::enable(GLenum planeId)
{
    _planeId = planeId;

    if (_active)
    {
        glEnable(_planeId);
        glClipPlane(planeId, _normal);
    }
}



// --------------------------------------

// if currentCp == newCp the active flag is toggled
void switchClipPlane(ClipPlane **currentClipPlane,
                     ClipPlane *newClipPlane)
{
    // update current clip plane
    if (*currentClipPlane)
    {
        (*currentClipPlane)->_visible = false;
        if (*currentClipPlane == newClipPlane)
        {
            // deactivate clip plane and deselect
            (*currentClipPlane)->_active = false;
            (*currentClipPlane) = NULL;
            return;
        }
    }
    // update new clip plane
    *currentClipPlane = newClipPlane;
    if (newClipPlane)
    {
        (*currentClipPlane)->_active = true;
        (*currentClipPlane)->_visible = true;
    }
}
