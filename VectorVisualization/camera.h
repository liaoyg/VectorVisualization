#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "transform.h"
#include "mmath.h"


#ifndef MOUSE_SCALE
#  define MOUSE_SCALE         1.7f
#endif

class Camera : public Transform
{
public:
    Camera(void);
    ~Camera(void);

    // setup OpenGL matrix state (projection and modelview) 
    // using camera parameters including gluPerspective
    void setCamera(void);

    void setNearClipPlane(const float nearClip) { _nearClip = nearClip; }
    const float getNearClipPlane(void) { return _nearClip; }
    void setFarClipPlane(const float farClip) { _farClip = farClip; }
    const float getFarClipPlane(void) { return _farClip; }

    void setFovy(const float fovy) { _fovy = fovy; }
    const float getFovy(void) { return _fovy; }

    // translate the camera in the x-y plane
    void translate(int xNew, int yNew, int xOld, int yOld);
    // translate the camera in the z axis (dolly)
    void dolly(int xNew, int yNew, int xOld, int yOld);

    void setPosition(const Vector3 &v) { _pos = v; }
    Vector3 getPosition(void) { return _pos; }

    // if enable==true use camera positions from a halton sequence on a 
    // unit sphere. The current camera state is stored and will be restored
    // if the halton sequence is disabled.
    void enableHaltonPos(bool enable);

    bool loadHaltonPositions(const char *fileName);

    // returns the current index of the halton sequence
    const int getSeqIndex(void) { return _seqIdx; }
    void resetSequence(void) { _seqIdx = 0; }

    // sets the camera to the next element of halton sequence
    // when the halton sequence is enabled
    void nextHaltonPos(void);


    // set a new camera position to v
    Vector3& operator=(Vector3 &v) { _pos = v; return _pos; }
    // transform the current position by vector v
    Vector3& operator+=(Vector3 &v) { _pos = Vector3_add(_pos, v); return _pos; }

protected:
private:
    Vector3 _pos;
    float _nearClip;
    float _farClip;
    float _fovy;

    // get camera positions on a unit sphere from a halton sequence
    bool _useHaltonSequence;
    // was the camera state previously stored?
    bool _camSaved;

    float *_sequence;
    int _seqIdx;
    int _seqLength;

    Quaternion _savedCam_internal;
    float _savedCam_dist;
};


#endif // _CAMERA_H_
