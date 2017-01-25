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
 * Filename: fpsCounter.cpp
 * 
 * $Id: fpsCounter.cpp,v 1.2 2008/05/09 08:23:24 falkmn Exp $ 
 */

#ifdef _WIN32
#  include <windows.h>
#endif

#include <float.h>
#include "types.h"
#include "timer.h"
#include "fpsCounter.h"


FPSCounter::FPSCounter(void) : _fps(0.0f),_maxFPSCurrent(0.0f),
                               _minFPSCurrent(FLT_MAX),_maxFPS(0.0f),
                               _minFPS(FLT_MAX),_avgFPS(0.0f),
                               _frames(0),_tStart(0.0)
{
}


FPSCounter::~FPSCounter(void)
{
}


void FPSCounter::reset(void)
{
    _fps = 0.0f;
    _maxFPSCurrent = 0.0f;
    _minFPSCurrent = FLT_MAX;

    _maxFPS = 0.0f;
    _minFPS = FLT_MAX;
    _avgFPS = 0.0f;

    _frames = 0;
    _tStart = 0.0;
}


// initialize timer for current frame
// to be called before each frame
void FPSCounter::frameStart(void)
{
    if (_frames % FPS_MAXFRAMES == 0)
        _tStart = timer();
}


// Update FPS counter (time measurement is done inhere)
// to be called after each frame
void FPSCounter::frameFinished(void)
{
    double tEnd;

    // increase frame counter
    ++_frames;

    // time measurement over FPS_MAXFRAMES frames
    if (_frames % FPS_MAXFRAMES == 0)
    {
        tEnd = timer();
        _fps = (float) (1000.0 * FPS_MAXFRAMES / (tEnd - _tStart));

        if (_fps < _minFPSCurrent)
            _minFPSCurrent = _fps;
        if (_fps > _maxFPSCurrent)
            _maxFPSCurrent = _fps;

        _fpsSum += _fps;

        // average frames every _maxFrameCount frames
        if (_frames > (_maxFrameCount - 1))
        {
            _maxFPS = _maxFPSCurrent;
            _minFPS = _minFPSCurrent;
            _avgFPS = _fpsSum / (float) _frames;

            _frames = 0;
            _maxFPSCurrent = 0.0f;
            _minFPSCurrent = FLT_MAX;
            _fpsSum = 0.0f;
        }
    }
}
