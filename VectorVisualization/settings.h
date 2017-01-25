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
 * Filename: settings.h
 * 
 * $Id: settings.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#ifdef _WIN32
#  include <windows.h>
#endif

#define _USE_MATH_DEFINES

#include <GL/gl.h>
#include "../graphics/transferEdit.h"
#include "../camera/transform.h"
#include "../camera/camera.h"
#include "dataSet.h"
#include "mmath.h"

class Settings
{
public:
    Settings(void);
    ~Settings(void);
};

struct UserSettings
{
    UserSettings(void)
    {
        sceneTranslation = Vector3_new(0.0f, 0.0f, 0.0f);
        camTransform.setDistance(3.5f);
        lightTransform.setDistance(4.0f);

        clipPlane[0].setColor(1.0f, 0.0f, 0.0f, 0.05f);
        clipPlane[1].setColor(0.0f, 1.0f, 0.0f, 0.05f);
        clipPlane[2].setColor(0.0f, 0.0f, 1.0f, 0.05f);

        volumeStepSize = 1.0f/128.0f;
        licSteps = 25;
        licStepWidth = 0.002f;
        noiseFreqScale = 2.0f;
        
        numShaderIter = 255;
        
        wireframe = false;
        drawLight = false;
    }

    // scene translation and rotation
    // TODO: zusammenfassen und durch Camera-Klasse ersetzen?
    // Camera cam;
    Vector3 sceneTranslation;
    Transform camTransform;

    // light parameters
    Transform lightTransform;

    ClipPlane clipPlane[3];

    // LIC params
    float volumeStepSize;  // float stepSize;
    int licSteps;
    float licStepWidth;
    float noiseFreqScale; // float freqScale;

    // max iterations in fragment shader
    int numShaderIter;

    bool wireframe;
    bool drawLight;

    // background scaling
    //float bgTexScale;
    // background gray value
    // unsigned char bgGrayVal;
};


struct AppSettings
{
    VolumeData volume;

    TransferEdit transferEdit;

    ClipPlane *currentClipPlane;

};

#endif // _SETTINGS_H_
