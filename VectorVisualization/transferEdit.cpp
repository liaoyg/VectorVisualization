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
 * Filename: transferEdit.cpp
 * 
 * $Id: transferEdit.cpp,v 1.2 2008/05/09 08:23:22 falkmn Exp $ 
 */

#ifdef _WIN32
#  include <windows.h>
#endif

#define _USE_MATH_DEFINES

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
//#include <GL/glu.h>

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "texture.h"
#include "mmath.h"
#include "imageUtils.h"
#include "dataSet.h"
#include "transferEdit.h"



const float TransferEdit::_tfColor[5][4] = { { 1.0f, 0.0, 0.0f, 1.0f },
                                             { 0.0f, 1.0, 0.0f, 1.0f },
                                             { 0.0f, 0.0, 1.0f, 1.0f },
                                             { 1.0f, 1.0, 1.0f, 1.0f },
                                             { 0.0f, 1.0, 1.0f, 1.0f } };


TransferEdit::TransferEdit(void) : _numEntries(256),_visible(false),
                                   _showHistogram(false),_focus(false),
                                   _winWidth(1),_winHeight(1),
                                   _sceneUpdate(false),
                                   _fileNameRGBA(NULL),
                                   _fileNameAlphaOpac(NULL)
{
    _tfData = new unsigned char[5*_numEntries];
//    _tfRGBA = new unsigned char[4*_numEntries];
//    _tfAlphaOpac = new unsigned char[2*_numEntries];
    _histogram = new unsigned char[_numEntries];

    // initialize transfer functions
    //   rgb = identity
    //   alpha + opac = translated identity
    for (int i=0; i<_numEntries; ++i)
    {
        for (int ch=0; ch<3; ++ch)
            _tfData[5*i+ch] = (unsigned char) i;
        for (int ch=3; ch<5; ++ch)
            _tfData[5*i+ch] = (unsigned char) ((i<20) ? 0 : i-20);
    }

    // initialize histogram
    memset(_histogram, 0, sizeof(unsigned char)*_numEntries);


    for (int i=0; i<5; ++i)
        _activeChannel[i] = (i>2);
    _offset[0] = 10;
    _offset[1] = 10;

    _texRGB.texUnit = GL_TEXTURE7_ARB;
    _texAlphaOpac.texUnit = GL_TEXTURE8_ARB;

    _texRGB.format = GL_RGBA;
    _texAlphaOpac.format = GL_LUMINANCE_ALPHA;
}


TransferEdit::~TransferEdit(void)
{
    delete [] _tfData;
    //delete [] _tfRGBA;
    //delete [] _tfAlphaOpac;
    delete [] _histogram;

    delete [] _fileNameRGBA;
    delete [] _fileNameAlphaOpac;
}


void TransferEdit::setTFFileNames(const char *fileName)
{
    char *t = NULL;

    delete [] _fileNameRGBA;
    delete [] _fileNameAlphaOpac;

    _fileNameRGBA = NULL;
    _fileNameAlphaOpac = NULL;

    if (!fileName)
    {
        return;
    }

    // try to find the period of the extension (which should be png)
    t = (char*)strrchr(fileName, '.');
    if (!t)
    {
        // no extension found, just append "_rgba.png" and "_alpha.png"
        _fileNameRGBA = new char[strlen(fileName)+10];
        _fileNameAlphaOpac = new char[strlen(fileName)+11];

        strcpy(_fileNameRGBA, fileName);
        strcat(_fileNameRGBA, "_rgba.png");

        strcpy(_fileNameAlphaOpac, fileName);
        strcat(_fileNameAlphaOpac, "_alpha.png");
    }
    else
    {
        // extension found, insert "_rgba" and "_alpha"
        _fileNameRGBA = new char[strlen(fileName)+6];
        _fileNameAlphaOpac = new char[strlen(fileName)+7];

        strncpy(_fileNameRGBA, fileName, (t-fileName));
        _fileNameRGBA[t-fileName] = '\0';
        strcat(_fileNameRGBA, "_rgba");
        strcat(_fileNameRGBA, t);

        strncpy(_fileNameAlphaOpac, fileName, (t-fileName));
        _fileNameAlphaOpac[t-fileName] = '\0';
        strcat(_fileNameAlphaOpac, "_alpha");
        strcat(_fileNameAlphaOpac, t);
    }
    //std::cout << "TransferEdit:   " << fileName << " -> " 
    //    << _fileNameRGBA << " , " << _fileNameAlphaOpac << std::endl;
}


bool TransferEdit::saveTF(const char *fileName)
{
    Image img;

    if (!fileName)
    {
        fprintf(stderr, "TransferEdit Save:  No filename set.\n");
        return false;
    }
    setTFFileNames(fileName);

    fprintf(stdout, "Storing Transfer Function:  %s, %s\n", 
        _fileNameRGBA, _fileNameAlphaOpac);

    // store 10 lines for better visualization ;-)
    img.imgData = new unsigned char[4*_numEntries * 10];
    img.width = _numEntries;
    img.height = 10;
    img.channel = 4;

    // store rgba channels
    for (int j=0; j<10; ++j)
    {
        for (int i=0; i<_numEntries; ++i)
        {
            img.imgData[4*(j*_numEntries+i)]   = _tfData[5*i];
            img.imgData[4*(j*_numEntries+i)+1] = _tfData[5*i+1];
            img.imgData[4*(j*_numEntries+i)+2] = _tfData[5*i+2];
            img.imgData[4*(j*_numEntries+i)+3] = _tfData[5*i+3];
        }
    }
    if (!pngWrite(_fileNameRGBA, &img))
    {
        fprintf(stderr, "TransferEdit:  Could not save RGBA transfer function "
            "(\"%s\").\n", _fileNameRGBA);
        return false;
    }

    // store alpha/opacity channels
    img.channel = 2;
    for (int j=0; j<10; ++j)
    {
        for (int i=0; i<_numEntries; ++i)
        {
            img.imgData[2*(j*_numEntries+i)]   = _tfData[5*i+3];
            img.imgData[2*(j*_numEntries+i)+1] = _tfData[5*i+4];
        }
    }
    if (!pngWrite(_fileNameAlphaOpac, &img))
    {
        fprintf(stderr, "TransferEdit:  Could not save alpha/opacity transfer function "
            "(\"%s\").\n", _fileNameAlphaOpac);
        return false;
    }

    delete [] img.imgData;

    return true;
}


bool TransferEdit::loadTF(const char *fileName)
{
    Image img;

    if (!fileName)
    {
        fprintf(stderr, "TransferEdit Load:  No filename set.\n");
        return false;
    }
    setTFFileNames(fileName);

    fprintf(stdout, "Importing Transfer Function:\n   %s\n   %s\n", 
        _fileNameRGBA, _fileNameAlphaOpac);

    if (!loadRGBATF() && !loadAlphaOpacTF())
        return false;

    _sceneUpdate = true;

    return true;
}


bool TransferEdit::loadRGBATF(void)
{
    Image img;

    if (!pngRead(_fileNameRGBA, &img))
    {
        fprintf(stderr, "TransferEdit:  Could not load RGBA channels "
            "(\"%s\").\n", _fileNameRGBA);
        return false;
    }

    if (img.width*img.height < _numEntries)
    {
        fprintf(stderr, "TransferEdit:  Transfer function (\"%s\") contains too "
            "little information (RGBA).\n", _fileNameRGBA);
        delete [] img.imgData;
        return false;
    }

    //fprintf(stdout, "    Loading %d channels of RGBA transfer function.\n", img.channel);

    if (img.channel < 3) 
    {
        // gray scale or gray scale with alpha (duplicate value for rgb)
        for (int i=0; i<_numEntries; ++i)
        {
            _tfData[5*i]   = img.imgData[i*img.channel];
            _tfData[5*i+1] = img.imgData[i*img.channel];
            _tfData[5*i+2] = img.imgData[i*img.channel];
            if (img.channel == 2)
                _tfData[5*i+3] = img.imgData[i*img.channel+1];
        }
    }
    else
    {
        // RGB or RGBA 
        for (int i=0; i<_numEntries; ++i)
        {
            _tfData[5*i]   = img.imgData[i*img.channel];
            _tfData[5*i+1] = img.imgData[i*img.channel+1];
            _tfData[5*i+2] = img.imgData[i*img.channel+2];
            if (img.channel == 4)
                _tfData[5*i+3] = img.imgData[i*img.channel+3];
        }
    }
    delete [] img.imgData;
    img.imgData = NULL;

    return true;
}


bool TransferEdit::loadAlphaOpacTF(void)
{
    Image img;

    if (!pngRead(_fileNameAlphaOpac, &img))
    {
        fprintf(stderr, "TransferEdit:  Could not load alpha/opacity channel "
            "(\"%s\").\n", _fileNameAlphaOpac);
        return false;
    }

    if (img.width*img.height < _numEntries)
    {
        fprintf(stderr, "TransferEdit:  Transfer function (\"%s\") contains too "
            "little information (alpha/opacicity).\n", _fileNameAlphaOpac);
        delete [] img.imgData;
        return false;
    }

    /*
    if (img.channel > 2) 
        fprintf(stdout, "    Loading only first two channels of %d of alpha/opacicity transfer function.\n", img.channel);
    else
        fprintf(stdout, "    Loading %d channels of alpha/opacicity transfer function.\n", img.channel);
    */

    // gray scale or gray scale with alpha
    for (int i=0; i<_numEntries; ++i)
    {
        _tfData[5*i+3]   = img.imgData[i*img.channel];
        if (img.channel == 2)
            _tfData[5*i+4] = img.imgData[i*img.channel+1];
    }

    delete [] img.imgData;
    img.imgData = NULL;

    return true;
}


void TransferEdit::computeHistogram(VolumeData *vd)
{
    int *histogram;
    int v;
    int size;
    int di;
    float maxLen = -1.0f;
    float *magnitude = NULL;

    // initialize histogram
    histogram = new int[_numEntries];
    memset(histogram, 0, sizeof(int)*_numEntries);

    switch (vd->dataType)
    {
    case DATRAW_UCHAR:
        if (vd->dataDim == 1)
        {
            // scalar data
            for (int i=0; i<vd->size[0]*vd->size[1]*vd->size[2]; ++i)
            {
                ++histogram[((unsigned char*) vd->data)[i]];
            }
        }
        else
        {
            // vector data
            unsigned char *data = (unsigned char*) vd->data;
            size = vd->size[0]*vd->size[1]*vd->size[2];
            magnitude = new float[size];

            // determine maximum magnitude of vector field
            for (int i=0; i<size; ++i)
            {
                magnitude[i] = sqrt((float)SQR(data[i*vd->dataDim]-128) 
                    + (float)SQR(data[i*vd->dataDim+1]-128) 
                    + (float)SQR(data[i*vd->dataDim+2]-128));
                if (maxLen < magnitude[i])
                    maxLen = magnitude[i];
            }

            // normalize magnitude and update histogram
            for (int i=0; i<size; ++i)
            {
                v = (int)(magnitude[i]/maxLen * UCHAR_MAX);
                ++histogram[(v>255) ? 255 : v];
            }
            delete [] magnitude;
        }
        break;
    case DATRAW_FLOAT:
        if (vd->dataDim == 1)
        {
            // scalar data
            for (int i=0; i<vd->size[0]*vd->size[1]*vd->size[2]; ++i)
            {
                v = (int)(((float*) vd->data)[i] * UCHAR_MAX);
                ++histogram[(v>255) ? 255 : v];
            }
        }
        else
        {
            // vector data
            float *data = (float*) vd->data;
            size = vd->size[0]*vd->size[1]*vd->size[2];
            magnitude = new float[size];

            // determine maximum magnitude of vector field
            for (int i=0; i<size; ++i)
            {
                magnitude[i] = sqrt(SQR(data[i*vd->dataDim]) 
                    + SQR(data[i*vd->dataDim+1]) 
                    + SQR(data[i*vd->dataDim+2]));
                if (maxLen < magnitude[i])
                    maxLen = magnitude[i];
            }

            // normalize magnitude and update histogram
            for (int i=0; i<size; ++i)
            {
                v = (int)(magnitude[i]/maxLen * UCHAR_MAX);
                ++histogram[(v>255) ? 255 : v];
            }
            delete [] magnitude;
        }
        break;
    default:
        fprintf(stderr, "TransferEdit:  Unknown data type for histogram.\n");
        break;
    }

    // normalize histogram
    di = 0;
    // find maximum occurence
    for (int i=1; i<256; ++i)
        di = (histogram[di] > histogram[i]) ? di : i;

    for (int i=0; i<256; ++i)
        _histogram[i] = (unsigned char)
            (log((float)histogram[i])*256.0f/log((float)histogram[di]));

    delete [] histogram;
}


/*
            // vector data
            for (int i=0; i<vd->size[0]*vd->size[1]*vd->size[2]; ++i)
            {
                ++histogram[static_cast<unsigned char*>(vd->data)[4*i+3]];
            }
        }
        break;
    case DATRAW_FLOAT:
        if (vd->dataDim == 1)
        {
            // scalar data
            for (int i=0; i<vd->size[0]*vd->size[1]*vd->size[2]; ++i)
            {
                v = (int)(static_cast<float*>(vd->data)[i] * UCHAR_MAX);
                ++histogram[(v>255) ? 255 : v];
            }
        }
        else
        {
            // vector data
            for (int i=0; i<vd->size[0]*vd->size[1]*vd->size[2]; ++i)
            {
                v = (int)(static_cast<float*>(vd->data)[4*i+3] * UCHAR_MAX);
                ++histogram[(v>255) ? 255 : v];
            }
            */

/*
void TransferEdit::createTextures(void)
{
}
*/


void TransferEdit::updateTextures(void)
{
    GLuint texId;
    int texSize = _numEntries;
    unsigned char *paddedData = NULL;
    
    // create texture ids
    if (_texRGB.id == 0)
    {
        glGenTextures(1, &texId);
        _texRGB.setTex(GL_TEXTURE_1D, texId, "TF_RGBA");
    }
    if (_texAlphaOpac.id == 0)
    {
        glGenTextures(1, &texId);
        _texAlphaOpac.setTex(GL_TEXTURE_1D, texId, "TF_AlphaOpac");
    }

#if FORCE_POWER_OF_TWO_TEXTURE == 1
    texSize = nextPowerTwo(_numEntries);
#endif
    _texRGB.width = texSize;
    _texAlphaOpac.width = texSize;

    paddedData = new unsigned char[4*texSize];
    memset(paddedData, 0, 4*texSize*sizeof(unsigned char));

    // first fill the rgba texture
    for (int i=0; i<_numEntries; ++i)
    {
        paddedData[4*i] = _tfData[5*i];
        paddedData[4*i+1] = _tfData[5*i+1];
        paddedData[4*i+2] = _tfData[5*i+2];
        paddedData[4*i+3] = _tfData[5*i+3];
    }
    glBindTexture(GL_TEXTURE_1D, _texRGB.id);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, texSize, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, paddedData);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);


    // fill the alpha opacity texture
    for (int i=0; i<_numEntries; ++i)
    {
        paddedData[2*i] = _tfData[5*i+3];
        paddedData[2*i+1] = _tfData[5*i+4];
    }
    glBindTexture(GL_TEXTURE_1D, _texAlphaOpac.id);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_LUMINANCE_ALPHA, texSize,  
                 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, paddedData);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    delete [] paddedData;
}


void TransferEdit::draw(void)
{
    if (!_visible)
        return;

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, (double)_winWidth, 0.0, (double)_winHeight, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef((GLfloat)_offset[0], (GLfloat)_offset[1], 0.0f);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glDisable(GL_DEPTH_TEST);

    // draw background
    glBegin(GL_QUADS);
    {
        glColor4f(0.0f, 0.0f, 0.0f, 0.2f);
        glVertex2i(0, 0);
        glVertex2i(_numEntries-1, 0);
        glVertex2i(_numEntries-1, _numEntries-1);
        glVertex2i(0, _numEntries-1);
    }
    glEnd();

    // draw histogram
    if (_showHistogram)
    {
        glColor4f(0.0f, 0.0f, 0.4f, 0.3f);
        glBegin(GL_QUADS);
        {
            for (int i=0; i<_numEntries; ++i)
            {
                glVertex2i(i,   0);
                glVertex2i(i+1, 0);
                glVertex2i(i+1, _histogram[i]);
                glVertex2i(i,   _histogram[i]);
            }
        }
        glEnd();
    }

    // draw small boxes for active channels
    glBegin(GL_QUADS);
    {
        for (int i=0; i<5; ++i) 
        {
            if (_activeChannel[i])
            {
                glColor4fv(_tfColor[i]);

                glVertex2i(2*i * TE_BOXSIZE + TE_BOXSIZE, 8);
                glVertex2i(2*i * TE_BOXSIZE + 2 * TE_BOXSIZE, 8);
                glVertex2i(2*i * TE_BOXSIZE + 2 * TE_BOXSIZE, 8 + TE_BOXSIZE);
                glVertex2i(2*i * TE_BOXSIZE + TE_BOXSIZE, 8 + TE_BOXSIZE);
            }
        }
    }
    glEnd();

    // draw tf functions as lines
    for (int j=0; j<5; ++j) 
    {
        glColor4fv(_tfColor[j]);
        glBegin(GL_LINE_STRIP);
        {
            for (int i=0; i<_numEntries; ++i)
                glVertex2i(i, _tfData[5*i+j]);
        }
        glEnd();
    }

    glDisable(GL_BLEND);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
}


bool TransferEdit::mouseTE(int x, int y)
{
    if (!_visible || !isMouseInside(x, _winHeight - y))
    {
        _focus = false;
        return false;
    }
    _mousePosOld[0] = x - _offset[0];
    _mousePosOld[1] = _winHeight - (y + _offset[1]);
    _focus = true;
    return true;
}


bool TransferEdit::motionTE(int x, int y)
{
    int mousePos[2];
    float m;
    int start[2];
    int deltaX;

    if (!(_visible && _focus))
        return false;

    mousePos[0] = (x - _offset[0] < 0) ? 0 
        : (x - _offset[0] > UCHAR_MAX) ? UCHAR_MAX : (x - _offset[0]);
    mousePos[1] = (_winHeight - (y + _offset[1]) < 0) ? 0 
        : (_winHeight - (y + _offset[1]) > UCHAR_MAX) ? UCHAR_MAX
        : (_winHeight - (y + _offset[1]));

    if (mousePos[0] < _mousePosOld[0])
    {
        start[0] = mousePos[0];
        start[1] = mousePos[1];
        deltaX = _mousePosOld[0] - mousePos[0];
        m = (_mousePosOld[1] - mousePos[1]) / (float) deltaX;
    }
    else
    {
        start[0] = _mousePosOld[0];
        start[1] = _mousePosOld[1];
        deltaX = mousePos[0]- _mousePosOld[0];
        m = (mousePos[1] - _mousePosOld[1]) / (float) deltaX;
    }

    for (int c=0; c<5; ++c)
    {
        if (_activeChannel[c])
        {
            for (int i=0; i<=deltaX; ++i)
                _tfData[5*(i+start[0])+c] = 
                    (unsigned char) (start[1] + (int)floor(i*m));
        }
    }

    _mousePosOld[0] = mousePos[0];
    _mousePosOld[1] = mousePos[1];

    return true;
}


bool TransferEdit::keyboardTE(unsigned key, int x, int y)
{
    if (!(_visible && isMouseInside(x, _winHeight - y)))
        return false;

    switch (key) 
    {
    case 'r':
        _activeChannel[0] = !_activeChannel[0];
        break;
    case 'g':
        _activeChannel[1] = !_activeChannel[1];
        break;
    case 'b':
        _activeChannel[2] = !_activeChannel[2];
        break;
    case 'a':
        _activeChannel[3] = !_activeChannel[3];
        break;
    case 'o':
        _activeChannel[4] = !_activeChannel[4];
        break;
    case 't':
        _visible = !_visible;
        break;
    case 'i':
        identityTF();
        break;
    case 'm':
        invertTF();
        break;
    case 'c':
        constTF();
        break;
    case 'h':
        shiftTF(TE_SHIFT_VAL_STEP);
        break;
    case 'n':
        shiftTF(-TE_SHIFT_VAL_STEP);
        break;
    case 's':
        _showHistogram = !_showHistogram;
        break;
    case 'p':
        updateTextures();
        _sceneUpdate = true;
        break;
        /*
    case 'L':
        break;
    case 'S':
        break;
    case 'W':
        fprintf(stderr, "Writing transfer function to: ");
        fflush(stderr);
        scanf("%s", &buf[0]);
        saveTransferFunction(buff);
        break;
    case 'R':
        fprintf(stderr, "Reading transfer function from: ");
        fflush(stderr);
        snscanf("%s", &buf[0]);
        loadTransferFunction(buff);
        break;
        */
    default:
        return false;
    }

    return true;
}


void TransferEdit::invertTF(void)
{
    for (int i=0; i < _numEntries; ++i)
        for (int j=0; j<5; ++j)
            if (_activeChannel[j]) 
                _tfData[5*i+j] = 255 - _tfData[5*i+j];
}


void TransferEdit::identityTF(void)
{
    for (int i=0; i < _numEntries; ++i)
        for (int j=0; j<5; ++j)
            if (_activeChannel[j]) 
                _tfData[5*i+j] = (unsigned char) i;
}


void TransferEdit::constTF(void)
{
    for (int i=0; i < _numEntries; ++i)
        for (int j=0; j<5; ++j)
            if (_activeChannel[j]) 
                _tfData[5*i+j] = (unsigned char) TE_CONST_VAL;
}


void TransferEdit::shiftTF(const int value)
{
    int temp;

    for (int i=0; i < _numEntries; ++i)
        for (int j=0; j<5; ++j)
            if (_activeChannel[j]) 
            {
                temp = _tfData[5*i+j] + value;
                _tfData[5*i+j] = (unsigned char) ((temp> 255) ? 255 : ((temp < 0) ? 0 : temp));
            }
}


bool TransferEdit::isMouseInside(const int x, const int y)
{
    return (((_offset[0] <= x) && (x <= _offset[0] + _numEntries)) 
        && ((_offset[1] <= y) && (y <= _offset[1] + _numEntries)));
}

