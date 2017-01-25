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
 * Filename: transferEdit.h
 * 
 * $Id: transferEdit.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _TRANSFEREDIT_H_
#define _TRANSFEREDIT_H_

#define _USE_MATH_DEFINES

#ifdef _WIN32
#  include <windows.h>
#endif

//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "texture.h"
#include "dataSet.h"


#define TE_BOXSIZE           8
#define TE_CONST_VAL        20
#define TE_SHIFT_VAL_STEP    4


class TransferEdit
{
public:
    TransferEdit(void);
    ~TransferEdit(void);

    void resize(const int width, const int height)
    {
        _winWidth = width;
        _winHeight = height;
    }

    bool saveTF(const char *fileName);
    bool loadTF(const char *fileName);

    void computeHistogram(VolumeData *vd);

    //void createTextures(void);
    void setTexUnits(GLuint rgbaTexUnit, 
                     GLuint alphaOpacTexUnit)
    {
        _texRGB.texUnit = rgbaTexUnit;
        _texAlphaOpac.texUnit = alphaOpacTexUnit;
    }
    void updateTextures(void);

    Texture* getTextureRGB(void) { return &_texRGB; }
    Texture* getTextureAlphaOpac(void) { return &_texAlphaOpac; }

    void setVisible(bool visible) { _visible = visible; }
    bool isVisible(void) { return _visible; }
    void toggleVisibility(void) { _visible = !_visible; }

    void setOffset(int offsetX, int offsetY)
    {
        _offset[0] = offsetX;
        _offset[1] = offsetY;
    }



    bool isSceneUpdateNeeded(void) 
    {
        if (_sceneUpdate)
        {
            _sceneUpdate = false;
            return true;
        }
        else
            return false;
    }

    int getNumEntries(void) { return _numEntries; }

    // draw transfer editor and transfer function
    void draw(void);

    // mouse handling
    bool mouseTE(int x, int y);
    bool motionTE(int x, int y);
    bool keyboardTE(unsigned key, int x, int y);

protected:
    bool loadRGBATF(void);
    bool loadAlphaOpacTF(void);

    void setTFFileNames(const char *fileName);                        
    void invertTF(void);
    void identityTF(void);
    void constTF(void);
    void shiftTF(const int value);

    bool isMouseInside(const int x, const int y);

private:
    const int _numEntries;

    // contains all channels of the transfer function
    unsigned char *_tfData;
    // contains RGB and alpha
    //unsigned char *_tfRGBA;
    // contains alpha (duplicated) and the additional LIC opacity
    //unsigned char *_tfAlphaOpac;

    unsigned char *_histogram;

    bool _visible;
    bool _activeChannel[5];
    bool _showHistogram;
    bool _focus;

    int _winWidth;
    int _winHeight;
    int _offset[2];

    bool _sceneUpdate;

    int _mousePosOld[2];

    static const float _tfColor[5][4];

    Texture _texRGB;
    Texture _texAlphaOpac;

    // when a filename is set, _fileNameRGBA contains the name of the
    // png file containing the RGBA channels of the transfer function
    char *_fileNameRGBA;
    // when a filename is set, _fileNameAlphaOpac contains the name of the
    // png file containing the alpha and opacity channel of the transfer 
    // function
    char *_fileNameAlphaOpac;
};

#endif // _TRANSFEREDIT_H_
