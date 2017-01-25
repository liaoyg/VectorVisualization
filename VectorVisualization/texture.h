#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#ifdef _WIN32
#  include <windows.h>
#endif

//#include <glh/glh_extensions.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
//#include <GL/gl.h>
#include <string.h>

/*
class Texture
{
public:
    Texture(void);
    ~Texture(void);
};
*/

struct Texture
{
    Texture(void) : texTarget(GL_TEXTURE_2D),format(0),id(0),
                    name(NULL),texUnit(GL_TEXTURE0_ARB),
                    width(1),height(1),depth(1)
    {
    }
    Texture(const Texture &tex) : texTarget(GL_TEXTURE_2D),format(0),id(0),
                                  name(NULL),texUnit(GL_TEXTURE0_ARB),
                                  width(1),height(1),depth(1)
    {
        setTexName(tex.name);
    }
    ~Texture(void) { delete [] name; }

    Texture& operator=(const Texture& tex)
    {
        texTarget = tex.texTarget;
        format = tex.format;
        id = tex.id;
        texUnit = tex.texUnit;

        setTexName(tex.name);

        return *this;
    }

    void setTex(GLenum target, GLuint texId, const char *name)
    {
        texTarget = target;
        id = texId;
        setTexName(name);
    }
    void bind(void)
    {
        //glEnable(texTarget);
        glActiveTextureARB(texUnit);
        glBindTexture(texTarget, id);
    }
    void unbind(void)
    {
        //glDisable(texTarget);
        glActiveTextureARB(texUnit);
        glBindTexture(texTarget, 0);
    }
    void setTexName(const char *name)
    {
        if (!name)
            return;
        delete [] this->name;
        this->name = new char[strlen(name)+1];
        strcpy(this->name, name);
    }

    GLenum texTarget;
    GLint format;
    GLuint id;
    char *name;

    GLuint texUnit;

    int width;
    int height;
    int depth;
};

#endif // _TEXTURE_H_
