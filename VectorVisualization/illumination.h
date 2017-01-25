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
 * Filename: illumination.h
 * 
 * $Id: illumination.h,v 1.2 2008/05/09 08:23:23 falkmn Exp $ 
 */
#ifndef _ILLUMINATION_H_
#define _ILLUMINATION_H_

#ifdef _WIN32
#include <windows.h>
#endif

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "mmath.h"
#include "texture.h"

struct LineMat
{
    LineMat(void)
    {
        lightColor[0] = lightColor[1] = 1.0f;
        lightColor[2] = lightColor[3] = 1.0f;
        ambient[0] = ambient[1] = 0.1f;
        ambient[2] = ambient[3] = 0.1f;
        diffuse[0] = diffuse[1] = 0.5f;
        diffuse[2] = diffuse[3] = 0.5f;
        specular[0] = specular[1] = 0.8f;
        specular[2] = specular[3] = 0.8f;
        specExp = 40.0f;
        diffExp = 2.0f;
    }

    float lightColor[4];
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float specExp;
    float diffExp;
};


class Illumination
{
public:
    Illumination(void);
    ~Illumination(void);

    void setTextureWidth(const unsigned int width) 
        { _texWidth = nextPowerTwo(width); }
    void setTextureHeight(const unsigned int height) 
        { _texHeight = nextPowerTwo(height); }

    void setMaterial(const LineMat *material);
    const LineMat getMaterial(void) { return _lineMat; }

    void setLightColor(float lightCol[]);
    void setAmbient(float ambient[]);
    void setDiffuse(float diffuse[]);
    void setSpecular(float specular[]);
    void setSpecularExp(const float specExp) { _lineMat.specExp = specExp; }
    void setDiffuseExp(const float diffExp) { _lineMat.diffExp = diffExp; }

    float getSpecularExp(void) { return _lineMat.specExp; }

    // calculate illumination textures according to Zoeckler and Mallo
    void createIllumTextures(bool zoeckler=true, bool mallo=true, bool floatTex=false);

    inline Texture* getTexZoeckler(void) { return &_texZoeckler; }
    inline Texture* getTexMalloDiffuse(void) { return &_texMalloDiffuse; }
    inline Texture* getTexMalloSpecular(void) { return &_texMalloSpecular; }

    // in debug mode the illumination textures are written 
    // to the file system as .png's during the creation
    void setDebugMode(bool enable) { _debug = enable; }

protected:
    // implementation of "Interactive Visualization of 3D-Vector Fields
    //                     Using Illuminated Stream Lines"
    //   by Zoeckler, Stalling, and Hege.  VIS 1996
    void createIllumTexZoeckler(bool floatTex=false);

    // implementation of "Illuminated Lines Revisited"
    //   by Mallo, Peikert, Sigg, and Sadlo.  VIS 2005
    void createIllumTexMallo(bool floatTex=false);

    void createTex(const GLuint texId, float *texData, GLenum texFormat, GLint texIntFormat);

    double computeSpecTermMallo(double alpha, double beta, double n);
    double computeSpecTermIntegrandMallo(double beta, double n, double theta);

private:
    LineMat _lineMat;

    Texture _texZoeckler;
    Texture _texMalloDiffuse;
    Texture _texMalloSpecular;

    int _texWidth;
    int _texHeight;

    bool _debug;
};

#endif // _ILLUMINATION_H_
