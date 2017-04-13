#pragma once

#ifndef TYPES_H_
#define TYPES_H_

//#ifdef _WIN32
//#  include <windows.h>
//#  define snprintf      _snprintf
//#endif

#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>
#include <GL\glew.h>
#include <GL\freeglut.h>

#define DIR_SEP_WIN '\\'
#define DIR_SEP '/'

#define DEBUG_OPENGL				1
#define FBO_USE_DEPTH_BUFFER        1
#define FBO_USE_STENCIL_BUFFER      0
#define FORCE_POWER_OF_TWO_TEXTURE  0

// some OpenGL defines
#define GL_DEPTH_STENCIL_EXT                              0x84F9
#define GL_UNSIGNED_INT_24_8_EXT                          0x84FA
#define GL_DEPTH24_STENCIL8_EXT                           0x88F0
#define GL_TEXTURE_STENCIL_SIZE_EXT                       0x88F1


#define SHADERS_DIR              "./shader/"
//#define BACKGROUND_IMAGE         "backgrounds/chess.ppm"

#define LOW_RES_TIMER_DELAY  0.5

#define VOL_FILE_EXT        ".dat"
#define SETTINGS_EXT        ".stg"
#define GRADIENTS_EXT       ".grd"
#define GRADIENTSFLT_EXT    ".grdf"

#define MAX_FRAMES            5

#define WINDOW_WIDTH        1280// 352  // 512
#define WINDOW_HEIGHT       960// 288  // 512

#define FOVY                30.0
#define NEAR_CLIP            0.001
#define FAR_CLIP             20.0

//#define SLICE_STEP           0.1
//#define MAX_SLICE_THICKNESS 10.0
//#define MIN_SLICE_THICKNESS SLICE_STEP

#define STEPSIZE_STEP        0.001
#define MAX_STEPSIZE         1.0
#define MIN_STEPSIZE         0.0

//#define TEX_COORD_SCALE_STEP 0.05


enum RenderTechnique
{
	VOLIC_VOLUME,
	VOLIC_RAYCAST,
	VOLIC_SLICING,
	//VOLIC_DEPTH_PEEL_MULTI
	VOLIC_LICVOLUME,
	VOLIC_VOLUMEANI,
};

enum MouseMode
{
	VOLIC_MOUSE_ROTATE,
	VOLIC_MOUSE_TRANSLATE,
	VOLIC_MOUSE_DOLLY,
	VOLIC_MOUSE_ROTATE_LIGHT,
	VOLIC_MOUSE_TRANSLATE_LIGHT,
	VOLIC_MOUSE_ROTATE_CLIP,
	VOLIC_MOUSE_TRANSLATE_CLIP
};


struct LICParams
{
	LICParams(void) : stepSizeVol(1.0f / 128.0f), gradientScale(30.0f),
		illumScale(1.0f), freqScale(1.0f),
		numIterations(255),
		stepsForward(32), stepsBackward(32),
		stepSizeLIC(0.01f)
	{}

	float stepSizeVol;
	float gradientScale;
	float illumScale;
	float freqScale;

	int numIterations;
	int stepsForward;
	int stepsBackward;
	float stepSizeLIC;
};


#if DEBUG_OPENGL == 1
#  define CHECK_FOR_OGL_ERROR()                                \
   do {                                                        \
     GLenum err;                                               \
     err = glGetError();                                       \
     if (err != GL_NO_ERROR)                                   \
     {                                                         \
       if (err == GL_INVALID_FRAMEBUFFER_OPERATION_EXT)        \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: Invalid Framebuffer Operation\n",\
                 __FILE__, __LINE__);                          \
       }                                                       \
       else                                                    \
       {                                                       \
         fprintf(stderr, "%s(%d) glError: %s\n",               \
                 __FILE__, __LINE__, gluErrorString(err));     \
       }                                                       \
     }                                                         \
   } while(0)

#  define CHECK_FRAMEBUFFER_STATUS()                                  \
   {                                                                  \
     GLenum status;                                                   \
     status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);        \
     switch(status) {                                                 \
        case GL_FRAMEBUFFER_COMPLETE_EXT:                             \
            break;                                                    \
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:                \
            printf("Framebuffer incomplete attachment\n");            \
            assert(0);                                                \
            break;                                                    \
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:        \
            printf("Framebuffer incomplete, missing attachment\n");   \
            assert(0);                                                \
            break;                                                    \
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:                \
            printf("Framebuffer incomplete, attached images "         \
                   "must have same dimensions\n");                    \
            assert(0);                                                \
            break;                                                    \
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:                   \
            printf("Framebuffer incomplete, attached images "         \
                   "must have same format\n");                        \
            assert(0);                                                \
            break;                                                    \
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:               \
            printf("Framebuffer incomplete, missing draw buffer\n");  \
            assert(0);                                                \
            break;                                                    \
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:               \
            printf("Framebuffer incomplete, missing read buffer\n");  \
            assert(0);                                                \
            break;                                                    \
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:                          \
            printf("Framebuffer unsupported\n");                      \
            assert(0);                                                \
            break;                                                    \
        default:                                                      \
            printf("Framebuffer: unknown error\n");                   \
            assert(0);                                                \
     }                                                                \
   }
/*
case GL_FRAMEBUFFER_STATUS_ERROR_EXT:                         \
printf("Framebuffer status error\n");                     \
break;                                                    \
*/
#else 
#  define CHECK_FOR_OGL_ERROR() 
#  define CHECK_FRAMEBUFFER_STATUS()
#endif

#endif // TYPES_H_
