/*
 * Copyright (c) 2008  Martin Falk <falk@visus.uni-stuttgart.de>
 *                     Visualization Research Center (VISUS), 
 *                     Universitšt Stuttgart, Germany
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
 * Filename: settings.cpp
 * 
 * $Id: settings.cpp,v 1.2 2008/05/09 08:23:24 falkmn Exp $ 
 */

#ifdef _WIN32
#  include <windows.h>
#endif

#define _USE_MATH_DEFINES

#include <GL/gl.h>
#include "mmath.h"
#include "settings.h"


Settings::Settings(void)
{
}


Settings::~Settings(void)
{
}
