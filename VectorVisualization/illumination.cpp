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
 * Filename: illumination.cpp
 * 
 * $Id: illumination.cpp,v 1.2 2008/05/09 08:23:22 falkmn Exp $ 
 */

#ifdef _WIN32
#  include <windows.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <limits.h>

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
//#include <GL/glu.h>

#include "texture.h"
#include "mmath.h"
#include "imageUtils.h"
#include "illumination.h"


Illumination::Illumination(void) : _texWidth(256),_texHeight(256),_debug(false)
{
}


Illumination::~Illumination(void)
{
    if (_texZoeckler.id)    
        glDeleteTextures(1, &_texZoeckler.id);
    if (_texMalloDiffuse.id)    
        glDeleteTextures(1, &_texMalloDiffuse.id);
    if (_texMalloSpecular.id)    
        glDeleteTextures(1, &_texMalloSpecular.id);
}


void Illumination::createIllumTextures(bool zoeckler, bool mallo, bool floatTex)
{
    GLuint texId;

    // setup textures and compute illumination textures
    if (zoeckler)
    {
        if (_texZoeckler.id == 0)
        {
            glGenTextures(1, &texId);
            _texZoeckler.setTex(GL_TEXTURE_2D, texId, "IllumTex_Zoeckler");
            _texZoeckler.texUnit = GL_TEXTURE9_ARB;
        }
        createIllumTexZoeckler();
    }
    if (mallo)
    {
        if (_texMalloDiffuse.id == 0)
        {
            glGenTextures(1, &texId);
            _texMalloDiffuse.setTex(GL_TEXTURE_2D, texId, "IllumTex_MalloDiff");
            _texMalloDiffuse.texUnit = GL_TEXTURE9_ARB;
        }
        if (_texMalloSpecular.id == 0)
        {
            glGenTextures(1, &texId);
            _texMalloSpecular.setTex(GL_TEXTURE_2D, texId, "IllumTex_MalloSpec");
            _texMalloSpecular.texUnit = GL_TEXTURE10_ARB;
        }
        createIllumTexMallo();
    }
}


// implementation of "Interactive Visualization of 3D-Vector Fields
//                     Using Illuminated Stream Lines"
//   by Zoeckler, Stalling, and Hege.  VIS 1996
void Illumination::createIllumTexZoeckler(bool floatTex)
{
    float *texData;
    double invResX, invResY;
    double t1, t2;
    double lt, vt;
    double dotproduct;
    double diffuse;
    double traditionalDiff;
    double traditionalSpec;
    int idx;

    texData = new float[2*_texWidth*_texHeight];
    _texZoeckler.width = _texWidth;
    _texZoeckler.height = _texHeight;

    _texZoeckler.format = (floatTex ? GL_LUMINANCE_ALPHA16F_ARB : GL_LUMINANCE_ALPHA);

    // illumination of lines according to Zoeckler et al. Vis96
    idx = 0;
    invResX = 1.0 / (double) (_texWidth - 1);
    invResY = 1.0 / (double) (_texHeight - 1);
    for (int y=0; y<_texHeight; ++y)
    {
        for (int x=0; x<_texWidth; ++x)
        {
            t1 = (double) x * invResX;
            t2 = (double) y * invResY;

            lt = 2.0*t1 - 1.0;
            vt = 2.0*t2 - 1.0;
            diffuse = sqrt(1.0 - lt*lt);
            diffuse = pow(diffuse, (double)_lineMat.diffExp); // trick by Zoeckler et al.

            dotproduct = lt*vt - sqrt(1.0 - lt*lt) * sqrt(1.0 - vt*vt);
            dotproduct = (dotproduct < -1.0) ? -1.0 : 
                ((dotproduct > 1.0) ? 1.0 : dotproduct);

            // use four components
            /*
            for (i=0; i<4; ++i)
            {
                traditionalDiff = _lineMat.ambient[i] * _lineMat.lightColor[i]
                    + diffuse * _lineMat.diffuse[i] * _lineMat.lightColor[i];

                traditionalSpec = pow(fabs(dotproduct), _lineMat.specExp) 
                    * _lineMat.specular[i] * _lineMat.lightColor[i];

                color = traditionalDiff + traditionalSpec;

                //        ic = (int) (127.0*color);
                //        ic = (ic < 0) ? 0 : ((ic > 255) ? 255 : ic);
                ic = 0.5*color;
                ic = (ic < 0.0) ? 0.0 : ((ic > 1.0) ? 1.0 : ic);

                illumTex[idx++] = ic;
            }
            */

            // separate diffuse to red channel, specular to green channel
            traditionalDiff = _lineMat.ambient[0] * _lineMat.lightColor[0]
                + diffuse * _lineMat.diffuse[0] * _lineMat.lightColor[0];

            traditionalSpec = pow(fabs(dotproduct), static_cast<double>(_lineMat.specExp)) 
                * _lineMat.specular[0] * _lineMat.lightColor[0];

            traditionalDiff *= 0.5;
            traditionalDiff = (traditionalDiff < 0.0) ? 0.0 
                : ((traditionalDiff > 1.0) ? 1.0 : traditionalDiff);

            traditionalSpec *= 0.5;
            traditionalSpec = (traditionalSpec < 0.0) ? 0.0 
                : ((traditionalSpec > 1.0) ? 1.0 : traditionalSpec);

            texData[idx++] = static_cast<float>(traditionalDiff);
            texData[idx++] = static_cast<float>(traditionalSpec)*0.9f;
        }
    }

    createTex(_texZoeckler.id, texData, GL_LUMINANCE_ALPHA, _texZoeckler.format);

    if (_debug)
    {
        // save texture to a image file
        Image img;

        img.width = _texWidth;
        img.height = _texHeight;
        img.channel = 3;
        img.imgData = new unsigned char[3*_texWidth*_texHeight];

        for (int i=0; i<_texWidth*_texHeight; ++i)
        {
            img.imgData[3*i] = (unsigned char) 
                (texData[2*i]*UCHAR_MAX);
            img.imgData[3*i+1] = (unsigned char) 
                (texData[2*i+1]*UCHAR_MAX);
            img.imgData[3*i+2] = (unsigned char) 0;
        }

        if (!pngWrite("img/illumTexZoeckler.png", &img, true))
        {
            fprintf(stderr, "Illumination:   Could not save Zoeckler "
                "texture to \"%s\".\n", "img/illumTexZoeckler.png");
        }
        else
        {
            fprintf(stdout, "Illumination:   Zoeckler "
                    "texture written to \"%s\".\n", "img/illumTexZoeckler.png");
        }
        delete [] img.imgData;
    }

    fprintf(stdout, "Zoeckler illumination texture created.\n");
    delete [] texData;
}


// implementation of "Illuminated Lines Revisited"
//   by Mallo, Peikert, Sigg, and Sadlo.  VIS 2005
void Illumination::createIllumTexMallo(bool floatTex)
{
    float *texDataDiff;
    float *texDataSpec;
    double alpha, beta;
    double lt;
    double color;
    double specular, diffuse;
    double s, t;
    int idx;

    texDataDiff = new float[4*_texWidth*_texHeight];
    texDataSpec = new float[4*_texWidth*_texHeight];

    _texMalloDiffuse.width = _texWidth;
    _texMalloDiffuse.height = _texHeight;
    _texMalloSpecular.width = _texWidth;
    _texMalloSpecular.height = _texHeight;

    _texMalloDiffuse.format = (floatTex ? GL_RGBA16F_ARB : GL_RGBA);
    _texMalloSpecular.format = (floatTex ? GL_RGBA16F_ARB : GL_RGBA);

    idx = 0;
    for (int y=0; y<_texHeight; ++y)
    {
        for (int x=0; x<_texWidth; ++x)
        {
            s = ((double) x + 0.5) / _texWidth;
            t = ((double) y + 0.5) / _texHeight;

            alpha = acos(2.0 * s - 1.0);
            beta = acos(2.0 * t - 1.0);
            lt = 2.0 * t - 1.0;

            // diffuse texture  F_d(cos(alpha), L_T) = 
            //     sqrt(1-L_T^2) * (sin(alpha) - (pi-alpha)cos(alpha) * 1/4)
            diffuse = sqrt(1.0 - lt*lt)
                * (sin(alpha) + (M_PI - alpha)*cos(alpha))*0.25;

            // specular texture F_s(cos(alpha), sin(beta)) =
            //     int_(alpha-pi/2)^(pi/2) cos^n(theta-beta) * cos(theta)*1/2 dtheta
            specular = 3.5*computeSpecTermMallo(alpha, beta, _lineMat.specExp);

            for (int i=0; i<4; ++i)
            {
                color = diffuse * _lineMat.diffuse[i] * _lineMat.lightColor[i];
                color = (color < 0.0) ? 0.0 : ((color > 1.0) ? 1.0 : color);
                texDataDiff[idx] = (float) color;

                color = specular * _lineMat.specular[i] * _lineMat.lightColor[i];
                color = (color < 0.0) ? 0.0 : ((color > 1.0) ? 1.0 : color);
                texDataSpec[idx] = (float) color;
                idx++;
            }
        }
    }

    createTex(_texMalloDiffuse.id, texDataDiff, GL_RGBA, _texMalloDiffuse.format);
    createTex(_texMalloSpecular.id, texDataSpec, GL_RGBA, _texMalloSpecular.format);

    if (_debug)
    {
        // save textures to a image file
        Image img;

        img.width = _texWidth;
        img.height = _texHeight;
        img.channel = 3;
        img.imgData = new unsigned char[3*_texWidth*_texHeight];

        // diffuse part
        for (int i=0; i<_texWidth*_texHeight; ++i)
        {
            img.imgData[3*i] = (unsigned char) 
                (texDataDiff[4*i]*UCHAR_MAX);
            img.imgData[3*i+1] = (unsigned char) 
                (texDataDiff[4*i+1]*UCHAR_MAX);
            img.imgData[3*i+2] = (unsigned char) 
                (texDataDiff[4*i+2]*UCHAR_MAX);
        }
        if (!pngWrite("img/illumTexMallo-diff.png", &img, true))
        {
            fprintf(stderr, "Illumination:   Could not save diffuse Mallo "
                "texture to \"%s\".\n", "img/illumTexMallo-diff.png");
        }
        else
        {
            fprintf(stdout, "Illumination:   Diffuse Mallo "
                    "texture written to \"%s\".\n", "img/illumTexMallo-diff.png");
        }

        // specular part
        for (int i=0; i<_texWidth*_texHeight; ++i)
        {
            img.imgData[3*i] = (unsigned char) 
                (texDataSpec[4*i]*UCHAR_MAX);
            img.imgData[3*i+1] = (unsigned char) 
                (texDataSpec[4*i+1]*UCHAR_MAX);
            img.imgData[3*i+2] = (unsigned char) 
                (texDataSpec[4*i+2]*UCHAR_MAX);
        }
        if (!pngWrite("img/illumTexMallo-spec.png", &img, true))
        {
            fprintf(stderr, "Illumination:   Could not save specular Mallo "
                "texture to \"%s\".\n", "img/illumTexMallo-spec.png");
        }
        else
        {
            fprintf(stdout, "Illumination:   Specular Mallo "
                    "texture written to \"%s\".\n", "img/illumTexMallo-spec.png");
        }
        delete [] img.imgData;
    }

    fprintf(stdout, "Mallo illumination textures created.\n");
    delete [] texDataDiff;
    delete [] texDataSpec;
}


void Illumination::createTex(const GLuint texId, float *texData,
                             GLenum texFormat, GLint texIntFormat)
{
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, texIntFormat, _texWidth, _texHeight, 
                 0, texFormat, GL_FLOAT, texData);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


double Illumination::computeSpecTermMallo(double alpha, double beta, double n)
{
    double	a, b;
    int		m, i;
    double	h;
    double	xi;
    double	integral;

    a = alpha - M_PI / 2.0;
    b = M_PI / 2.0;
    m = 10;
    h = (b - a) / (2.0 * m);

    integral = 0.0;
    for (i = 0; i < 2 * m; i += 2)
    {
        xi = a + i * h;
        integral += 2.0 * computeSpecTermIntegrandMallo(beta, n, xi);
        xi = a + (i + 1) * h;
        integral += 4.0 * computeSpecTermIntegrandMallo(beta, n, xi);
    }
    integral += computeSpecTermIntegrandMallo(beta, n, b);

    /* f(a) has been accounted for twice inside the for-loop. */
    integral -= computeSpecTermIntegrandMallo(beta, n, a);
    integral *= h / 3.0;

    return (integral);
}


double Illumination::computeSpecTermIntegrandMallo(double beta, double n, double theta)
{
    double y = cos(theta - beta);
    if (y < 0.0)
        y = 0.0;

    return (pow(y, n) * (cos(theta) / 2.0));
}

