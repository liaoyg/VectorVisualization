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
 * Filename: hud.h
 * 
 * $Id: hud.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _OPENGL_HUD_H_
#define _OPENGL_HUD_H_

#ifdef _WIN32
#  include <windows.h>
#endif

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

struct Color
{
    Color(void) : _r(0.0f),_g(0.0f),_b(0.0f),_a(0.0f) {}
    Color(float r, float g, float b, float a=1.0f)
    {
        _r = r; _g = g; _b = b; _a = a;
    }
    void Set(float r, float g, float b, float a=1.0f)
    {
        _r = r; _g = g; _b = b; _a = a;
    }
    void Set(Color &col)
    {
        _r = col._r;
        _g = col._g;
        _b = col._b;
        _a = col._a;
    }

    float _r;
    float _g;
    float _b;
    float _a;
};

class OpenGLHUD
{
public:
    OpenGLHUD(bool useVBO=false, bool nonPowerOfTwoTex=true);
    ~OpenGLHUD(void);

    void SetVisible(bool state=true) { _visible = state; }
    bool IsVisible(void) { return _visible; }

    // setup texture and vertexbuffer
    // returns false if not succesful
    bool Init(void);

    // set viewport for HUD (startX, startY, width, height)
    void SetViewport(int *viewport, bool forceUpdate=false);

    // set horizontal indentation
    void SetIndent(int indent, bool forceUpdate=false);
    int GetIndent(void) { return _indent; }
    // set the number of visible lines
    void SetNumLines(int numLines, bool forceUpdate=false);
    // set displayed text
    void SetText(const char *str, bool forceUpdate=false);

    // update the static text in the HUD (drawing only in FBO)
    void UpdateHUD(const char *str);
    // update HUD according to indentation/size/color
    void UpdateHUD(void);

    // draw HUD and add dynamic text (like fps count) at pos
    //   posX refers to indent in pixel,
    //   posY to vertical space in lines
    // The position is not checked whether it is inside the HUD
    void DrawHUD(const char *dynStr=NULL, unsigned int posX=0, unsigned int posY=0);

    // set font color of static text
    void SetForegroundColor(float r, float g, float b, float a, 
                            bool forceUpdate=false);
    void SetForegroundColor(Color &color, bool forceUpdate=false);
    // set font color of dynamic text
    void SetDynamicColor(float r, float g, float b, float a, bool forceUpdate);
    void SetDynamicColor(Color &color, bool forceUpdate);

    // set background color (top left, top right, bottom right, bottom left)
    void SetBackgroundColor(float r0, float g0, float b0, float a0,
                            float r1, float g1, float b1, float a1,
                            float r2, float g2, float b2, float a2,
                            float r3, float g3, float b3, float a3, 
                            bool forceUpdate=false);
    void SetBackgroundColor(Color &topLeft, Color &topRight, 
                            Color &bottomRight, Color &bottomLeft,
                            bool forceUpdate=false);

    GLuint GetHUDTexId(void) { return _hudTexId; }
    GLenum GetHUDTexTarget(void) { return _texTarget; }

protected:
    int GetNextPowerOfTwo(int n);
    // display each character of the string with glutBitmapCharacter
    // \n causes a newline
    void PrintString(void *font, const char *str,
                     int rasterPosX, int rasterPosY);

private:
    char *_hudStr; // can contain \n for multiline text

    // hud texture
    GLuint _hudTexId;
    unsigned int _texDim[2];
    GLenum _texTarget;

    unsigned _hudHeight;
    int _indent;
    int _numLines;
    int _linePitch; 

    // id of framebuffer object (for updating HUD texture)
    GLuint _fboId;
    // id of vertex buffer (4 vertices)
    GLuint _vboId;

    // vertices of HUD (2D)
    int _vertices[8];
    int _texCoord[8];

    // foreground color (font)
    Color _fgColor;
    // color of dynamic text (font)
    Color _dynColor;
    // colors (rgba) of the 4 vertices 
    // (left top, left bottom, right bottom, right top)
    Color _bgColor[4];
    
    // viewport
    int _viewport[4];

    bool _initialized;
    bool _useVBO;
    bool _nonPowerOfTwoTex;
    bool _visible;
    bool _changed;
};

#endif // _OPENGL_HUD_H_
