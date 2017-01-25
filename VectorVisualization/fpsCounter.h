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
 * Filename: fpsCounter.h
 * 
 * $Id: fpsCounter.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _FPSCOUNTER_H_
#define _FPSCOUNTER_H_

#include <iostream>
#include <iomanip>
#include "types.h"

#ifndef FPS_MAXFRAMES
#  define FPS_MAXFRAMES   5
#endif


class FPSCounter
{
public:
    FPSCounter(void);
    ~FPSCounter(void);

    void reset(void);

    // initialize timer for current frame
    // to be called before each frame
    void frameStart(void);
    // Update FPS counter (time measurement is done inhere)
    // to be called after each frame
    void frameFinished(void);

    float getFPS(void) { return _fps; }
    float getMaxFPS(void) { return _maxFPS; }
    float getMinFPS(void) { return _minFPS; }
    float getAvgFPS(void) { return _avgFPS; }

    unsigned int getFrameCount(void) { return _frames; }

    void setMaxFrameCount(unsigned int maxCount) { _maxFrameCount = maxCount; }

    friend std::ostream& operator<<(std::ostream &str, FPSCounter &fpsCounter)
    {
        return str << std::setprecision(3) << std::fixed 
                   << std::setw(7) << std::right
                   << "FPS min:  " << fpsCounter._minFPS << std::endl
                   << "FPS avg:  " << fpsCounter._avgFPS << std::endl
                   << "FPS max:  " << fpsCounter._maxFPS << std::endl
                   << "current:  " << fpsCounter._fps << std::endl;
    }

protected:
private:

    float _fps;
    float _maxFPSCurrent;
    float _minFPSCurrent;

    float _maxFPS;
    float _minFPS;
    float _avgFPS;

    int _frames;
    float _fpsSum;

    int _maxFrameCount;
    
    double _tStart;
};

#if 0
typedef struct FpsMeasure
{
  float min;
  float sum;
  float max;
  int frames;  // times MAX_FRAMES frames
  float fps;
  int frameCount_internal;
  int frameLimit;  // times MAX_FRAMES frames
} FpsMeas
#endif

#endif // _FPSCOUNTER_H_
