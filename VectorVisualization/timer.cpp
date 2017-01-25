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
 * Filename: timer.cpp
 * 
 * $Id: timer.cpp,v 1.2 2008/05/09 08:23:24 falkmn Exp $ 
 */

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/time.h>
#endif
#include <stdio.h>
#include "timer.h"

#ifdef _WIN32
double timer()
{
    LARGE_INTEGER timerFreq, timerCount;

    if (! QueryPerformanceFrequency(&timerFreq)) {
        fprintf(stderr, "Determing timer frequency failed\n");
        exit(1);
    }
    QueryPerformanceCounter(&timerCount);

    return (timerCount.QuadPart * 1000.0)/timerFreq.QuadPart;
}
#else
double timer()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec * 1e6 + t.tv_usec)/1000.0;
}
#endif
