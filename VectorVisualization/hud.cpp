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
 * Filename: hud.cpp
 * 
 * $Id: hud.cpp,v 1.4 2008/05/13 09:15:08 falkmn Exp $ 
 */

#ifdef _WIN32
#  include <windows.h>
#endif

#define _USE_MATH_DEFINES

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//
//#include <GL/gl.h>
//#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "texture.h"
#include "imageUtils.h"
#include "hud.h"


OpenGLHUD::OpenGLHUD(bool useVBO, bool nonPowerOfTwoTex)
{
    _hudStr = NULL;
    _hudTexId = 0;

    _indent = 10;
    _numLines = 2;
    _linePitch = 16;

    _fboId = 0;
    _vboId = 0;

    _fgColor.Set(1.0f, 1.0f, 0.0f, 1.0f);
    _dynColor.Set(1.0f, 1.0f, 1.0f, 1.0f);
    _bgColor[0].Set(0.0f, 0.2f, 0.6f, 0.7f);
    _bgColor[1].Set(0.0f, 0.2f, 0.4f, 0.7f);
    _bgColor[2].Set(0.0f, 0.4f, 0.2f, 0.7f);
    _bgColor[3].Set(0.0f, 0.3f, 0.5f, 0.7f);

    // _texCoord[8] = { 0, 1,   0, 0,   1, 0,   1, 1 };
    _texCoord[0] = _texCoord[2] = _texCoord[3] = _texCoord[5] = 0;
    _texCoord[1] = _texCoord[4] = _texCoord[6] = _texCoord[7] = 1;


    _viewport[0] = _viewport[1] = -1;

    _initialized = false;
    _visible = true;
    _useVBO = useVBO;
    _nonPowerOfTwoTex = nonPowerOfTwoTex;

    if (_nonPowerOfTwoTex)
        _texTarget = GL_TEXTURE_RECTANGLE_ARB;
    else
        _texTarget = GL_TEXTURE_2D;

    _changed = true;
}


OpenGLHUD::~OpenGLHUD(void)
{
    delete [] _hudStr;

    glDeleteTextures(1, &_hudTexId);
    if (glDeleteFramebuffersEXT)
        glDeleteFramebuffersEXT(1, &_fboId);
    if (_useVBO && glDeleteBuffersARB)
        glDeleteBuffersARB(1, &_vboId);
}


// setup texture and vertexbuffer
// returns false if not succesful
bool OpenGLHUD::Init(void)
{
    if ((_useVBO && !glGenBuffersARB) // no support for vertex buffer object
        || (!glGenFramebuffersEXT)) // no fbo extension
        return false;

    if (_initialized) // already done
        return true;

    // generate HUD texture
    glGenTextures(1, &_hudTexId);

    // generate framebuffer object
    glGenFramebuffersEXT(1, &_fboId);

    // generate vertex buffer object
    if (_useVBO)
    {
        glGenBuffersARB(1, &_vboId);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vboId);
        // 2x int (pos) + 2x int (texcoord) + 4x float (color) per Vertex
        // (non-interleaved)
        glBufferDataARB(GL_ARRAY_BUFFER, 4*(4*sizeof(int)+4*sizeof(float)),
                        NULL, GL_STATIC_DRAW_ARB);
    }

    _initialized = true;
    return true;
}


// set viewport for HUD (startX, startY, width, height)
void OpenGLHUD::SetViewport(int *viewport, bool forceUpdate)
{
    if (!viewport)
        return;

    if ((_viewport[0] == viewport[0])
        && (_viewport[1] == viewport[1])
        && (_viewport[2] == viewport[2])
        && (_viewport[3] == viewport[3]))
        return;

    _viewport[0] = viewport[0];
    _viewport[1] = viewport[1];
    _viewport[2] = viewport[2];
    _viewport[3] = viewport[3];

    _changed = true;

    if (forceUpdate)
        UpdateHUD();
}


// set horizontal indentation
void OpenGLHUD::SetIndent(int indent, bool forceUpdate)
{
    _indent = indent;

    _changed = true;

    if (forceUpdate && _hudStr)
        UpdateHUD();
}


// sets the number of visible lines
void OpenGLHUD::SetNumLines(int numLines, bool forceUpdate)
{
    if (numLines < 1)
        numLines = 1;

    _numLines = numLines;

    _changed = true;

    if (forceUpdate)
        UpdateHUD();
}


void OpenGLHUD::SetText(const char *str, bool forceUpdate)
{
    if (!str)        return;

    delete [] _hudStr;

    _hudStr = new char[strlen(str)+1];

    strcpy(_hudStr, str);

    _changed = true;
    if (forceUpdate)
        UpdateHUD();
}


// update the static text in the HUD (drawing only in FBO)
void OpenGLHUD::UpdateHUD(const char *str)
{
    if (!str)
        return;

    delete [] _hudStr;

    _hudStr = new char[strlen(str)+1];

    strcpy(_hudStr, str);

    _changed = true;
    UpdateHUD();
}


// update HUD according to indentation/size/color
void OpenGLHUD::UpdateHUD(void)
{
    float color[4];
    int oldViewport[4];
    int currentFBO = 0;
    int v[8];
    void *vertexPointer = NULL;
    float *colorPointer = NULL;

    if (_viewport[0] < 0) // viewport has not been set
    {
        std::cerr << "OpenGLHUD::UpdateHUD:  Viewport has not been set!"
            << std::endl;
        return;
    }


    // compute texture dimension and prepare vertices
    _texDim[0] = _viewport[2];
    _hudHeight = 14 + (_numLines-1)*_linePitch + 9;
    _texDim[1] = _hudHeight;
    if (!_nonPowerOfTwoTex)
    {
        _texDim[0] = GetNextPowerOfTwo(_texDim[0]);
        _texDim[1] = GetNextPowerOfTwo(_texDim[1]);
    }

    glGetIntegerv(GL_VIEWPORT, oldViewport);
    glViewport(0, 0, _texDim[0], _texDim[1]);


    // add viewport origin/size to the vertices for the final HUD
    // top left
    v[0] = 0;
    v[1] = _texDim[1];
    _vertices[0] = _viewport[0];
    _vertices[1] = _viewport[1] + _viewport[3];
    // bottom left
    v[2] = 0;
    v[3] = _texDim[1] - _hudHeight;
    _vertices[2] = _viewport[0];
    _vertices[3] = _viewport[1] + _viewport[3] - _hudHeight;
    // bottom right
    v[4] = _texDim[0];
    v[5] = _texDim[1] - _hudHeight;
    _vertices[4] = _viewport[0] + _viewport[2];
    _vertices[5] = _viewport[1] + _viewport[3] - _hudHeight;
    // top right
    v[6] = _texDim[0];
    v[7] = _texDim[1];
    _vertices[6] = _viewport[0] + _viewport[2];
    _vertices[7] = _viewport[1] + _viewport[3];

    if (_useVBO)
    {
        // prepare vertex buffer object
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vboId);
        // fill buffer non-interleaved (pos, tex, color)
        vertexPointer = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);

        // position
        for (int i=0; i<8; ++i)
        {
            ((int*)vertexPointer)[i] = _vertices[i];
        }
        // texcoords
        for (int i=0; i<8; ++i)
        {
            ((int*)vertexPointer)[i] = _texCoord[i];
        }
        // color
        colorPointer = (float*)&((int*)vertexPointer)[16];
             //= (float*) (vertexPointer + 16*sizeof(int))
        for (int i=0; i<4; ++i)
        {
            colorPointer[4*i]   = _bgColor[i]._r;
            colorPointer[4*i+1] = _bgColor[i]._g;
            colorPointer[4*i+2] = _bgColor[i]._b;
            colorPointer[4*i+3] = _bgColor[i]._a;
        }
        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
    }

    // prepare HUD texture
    glBindTexture(_texTarget, _hudTexId);
    glTexImage2D(_texTarget, 0, GL_RGBA, _texDim[0], _texDim[1],
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                    GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    // store current framebuffer object
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &currentFBO);

    // prepare framebuffer object
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fboId);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              _texTarget, _hudTexId, 0);


    // update HUD texture
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // store old clear color
    glGetFloatv(GL_COLOR_CLEAR_VALUE, color);

    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    glOrtho (-0.5, _texDim[0], -0.5, _texDim[1], -1.0, 1.0);
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();

    // draw background
    glBegin(GL_QUADS);
    {
        for (int i=0; i<4; ++i)
        {
            glColor4f(_bgColor[i]._r, _bgColor[i]._g, _bgColor[i]._b, _bgColor[i]._a);
            glTexCoord2iv(&_texCoord[2*i]);
            glVertex2iv(&v[2*i]);
        }
    }
    glEnd();

    // set foreground color
    glColor4f(_fgColor._r, _fgColor._g, _fgColor._b, _fgColor._a);

    // draw text
    if (_hudStr)
        PrintString(GLUT_BITMAP_HELVETICA_12, _hudStr, _indent, _texDim[1] - 14);

    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              _texTarget, 0, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, currentFBO);


    // DEBUG
    /*
    Image img;

    img.width = _texDim[0];
    img.height = _texDim[1];
    img.channel = 4;
    img.imgData = new unsigned char[4*_texDim[0]*_texDim[1]];

    glGetTexImage(_texTarget, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.imgData);

    std::cout << "Writing HUD.ppm\n" << std::endl;
    pngWrite("hud.png", &img);
    std::cout << "done" << std::endl;
    */
    // --------

    glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);

    // restore old clear color
    glClearColor(color[0], color[1], color[2], color[3]);

    _changed = false;
}


// draw HUD and add dynamic text (like fps count) at pos
//   posX refers to indent in pixel,
//   posY to vertical space in lines
// The position is not checked whether it is inside the HUD
void OpenGLHUD::DrawHUD(const char *dynStr, unsigned int posX, unsigned int posY)
{
    if (!_visible)
        return;

    if (_changed)
    {
        UpdateHUD();
    }

    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-0.5, _viewport[2], -0.5, _viewport[3], -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glActiveTextureARB(GL_TEXTURE0);
    glBindTexture(_texTarget, _hudTexId);
    glEnable(_texTarget);

    if (_useVBO)
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vboId);

        // number components, type, stride (bytes), offset
        glColorPointer(4, GL_FLOAT, 0, (float*)NULL + 16*sizeof(GL_INT));
        glTexCoordPointer(2, GL_INT, 0, (int*)NULL + 8*sizeof(GL_INT));
        glVertexPointer(2, GL_INT, 0, 0);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glDrawArrays(GL_QUADS, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    else
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glBegin(GL_QUADS);
        {
            for (int i=0; i<4; ++i)
            {
                //glColor4f(_bgColor[i]._r, _bgColor[i]._g, _bgColor[i]._b, _bgColor[i]._a);
                if (_nonPowerOfTwoTex)
                    glTexCoord2i(_texCoord[2*i]*_texDim[0], _texCoord[2*i+1]*_texDim[1]);
                else
                    glTexCoord2iv(&_texCoord[2*i]);
                glVertex2iv(&_vertices[2*i]);
            }
        }
        glEnd();

    }

    glDisable(_texTarget);
    
    // add dynamic text
    if (dynStr)
    {
        glColor4f(_dynColor._r, _dynColor._g, _dynColor._b, _dynColor._a);
        PrintString(GLUT_BITMAP_HELVETICA_12, dynStr, posX+_indent, 
                    _viewport[3] - (14+(posY-1)*_linePitch));
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}


// set font color
void OpenGLHUD::SetForegroundColor(float r, float g, float b, float a,
                        bool forceUpdate)
{
    _fgColor._r = r;
    _fgColor._g = g;
    _fgColor._b = b;
    _fgColor._a = a;

    _changed = true;

    if (forceUpdate)
        UpdateHUD();
}


void OpenGLHUD::SetForegroundColor(Color &color, bool forceUpdate)
{
    _fgColor._r = color._r;
    _fgColor._g = color._g;
    _fgColor._b = color._b;
    _fgColor._a = color._a;

    _changed = true;

    if (forceUpdate)
        UpdateHUD();
}


// set font color
void OpenGLHUD::SetDynamicColor(float r, float g, float b, float a,
                                bool forceUpdate)
{
    _dynColor._r = r;
    _dynColor._g = g;
    _dynColor._b = b;
    _dynColor._a = a;
}


void OpenGLHUD::SetDynamicColor(Color &color, bool forceUpdate)
{
    _dynColor._r = color._r;
    _dynColor._g = color._g;
    _dynColor._b = color._b;
    _dynColor._a = color._a;
}


// set background color (top left, top right, bottom right, bottom left)
void OpenGLHUD::SetBackgroundColor(float r0, float g0, float b0, float a0,
                                   float r1, float g1, float b1, float a1,
                                   float r2, float g2, float b2, float a2,
                                   float r3, float g3, float b3, float a3,
                                   bool forceUpdate)
{
    _bgColor[0].Set(r0, g0, b0, a0);
    _bgColor[1].Set(r3, g3, b3, a3);
    _bgColor[2].Set(r2, g2, b2, a2);
    _bgColor[3].Set(r1, g1, b1, a1);

    _changed = true;

    if (forceUpdate)
        UpdateHUD();
}


void OpenGLHUD::SetBackgroundColor(Color &topLeft, Color &topRight,
                                   Color &bottomRight, Color &bottomLeft,
                                   bool forceUpdate)
{
    _bgColor[0].Set(topLeft);
    _bgColor[1].Set(bottomLeft);
    _bgColor[2].Set(bottomRight);
    _bgColor[3].Set(topRight);

    _changed = true;

    if (forceUpdate)
        UpdateHUD();
}


int OpenGLHUD::GetNextPowerOfTwo(int n)
{
    int i = 1;
    while (i < n)
        i <<= 1;
    return i;
}


// display each character of the string with glutBitmapCharacter
// \n causes a newline
void OpenGLHUD::PrintString(void *font, const char *str, int rasterPosX, int rasterPosY)
{
    int len,i;
    int line = 0;

    len = (int)strlen(str);

    glRasterPos2i(rasterPosX, rasterPosY);
    for(i=0; i<len; i++)
    {
        if (str[i] == '\n')
        {
            ++line;
            //if (line > _numLines-1)
            //    break;
            glRasterPos2i(rasterPosX, rasterPosY - line*_linePitch);
        }
        else
        {
            glutBitmapCharacter(font, str[i]);
        }
    }
}

