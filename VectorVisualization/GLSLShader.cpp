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
 * Filename: GLSLShader.cpp
 * 
 * $Id: GLSLShader.cpp,v 1.2 2008/05/09 08:23:22 falkmn Exp $ 
 */

#ifdef _WIN32
#  include <windows.h>
#endif

//#define GLX_GLXEXT_PROTOTYPES
//#include <glh/glh_extensions.h>
//#include <GL/gl.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "types.h"
#include "GLSLShader.h"


GLSLShader::GLSLShader(void) : _programObj(0),_vertexShaderObj(0),
                               _fragmentShaderObj(0),_initialized(false)
{
}


GLSLShader::~GLSLShader(void)
{
    if (glDeleteObjectARB)
    {
        glDeleteObjectARB(_vertexShaderObj);
        glDeleteObjectARB(_fragmentShaderObj);
        glDeleteObjectARB(_programObj);
    }
}


bool GLSLShader::loadShader(int countVS, char **vertexShaderNames, 
                            int countFS, char **fragmentShaderNames,
                            char *defines)
{
    GLint compiled;
    char **shaderSrc = NULL;
    char *definesStr = NULL;
	char *versionStr = NULL;
    int numShaders = 0;


    if (!_initialized)
    {
        _vertShaderAttached = false;
        _fragShaderAttached = false;

        // create program object
        _programObj = glCreateProgramObjectARB();
        if (_programObj == 0)
        {
            std::cerr << "GLSLShader:  Could not create program object.\n";
            return false;
        }

        _vertexShaderObj = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
        if (_vertexShaderObj == 0)
        {
            std::cerr << "GLSLShader:  Could not create vertex shader object.\n";
            return false;
        }

        _fragmentShaderObj = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
        if (_fragmentShaderObj == 0)
        {
            std::cerr << "GLSLShader:  Could not create fragment shader object.\n";
            return false;
        }

        _initialized = true;
    }
    else
    {
        if (_fragShaderAttached)
        {
            _fragShaderAttached = false;
            glDetachObjectARB(_programObj, _fragmentShaderObj);
            CHECK_FOR_OGL_ERROR();
        }
        if (_vertShaderAttached)
        {
            _vertShaderAttached = false;
            glDetachObjectARB(_programObj, _vertexShaderObj);
            CHECK_FOR_OGL_ERROR();
        }
    }

    // structure defines into '#define name\n'
    if (defines)
    {
        int len = static_cast<int>(strlen(defines));
        definesStr = new char[len+2];
        strcpy(definesStr, defines);
        if (definesStr[len] != '\n')
        {
            definesStr[len] = '\n';
            definesStr[len+1] = '\0';
        }
    }
    else
    {
        definesStr = new char[1];
        definesStr[0] = '\0';
    }

	versionStr = new char[128];
	snprintf(versionStr, 128, "#version 120\n");

    // load vertex shader source files
    if (countVS > 0)
    {
        numShaders = countVS + 2;
        shaderSrc = new char*[numShaders];

		shaderSrc[0] = new char[strlen(versionStr) + 1];
		strcpy(shaderSrc[0], versionStr);

        shaderSrc[1] = new char[strlen(definesStr)+1];
        strcpy(shaderSrc[1], definesStr);
        
        for (int i=0; i<countVS; ++i)
        {
            //shaderSrc[2*i+1] = new char[128];
            //snprintf(shaderSrc[2*i+1], 128, "#line 0 %d\n", i);
            shaderSrc[i+2] = loadSource(vertexShaderNames[i]);
        }
		//shaderSrc = new char*[countVS];

		//for (int i = 0; i<countVS; ++i)
		//{
		//	shaderSrc[i] = loadSource(vertexShaderNames[i]);
		//}

        // load source into shader object
        glShaderSourceARB(_vertexShaderObj, numShaders,
                          (const GLcharARB**) shaderSrc, NULL);

        // compile shader
        glCompileShaderARB(_vertexShaderObj);

        for (int i=0; i<numShaders; ++i)
            delete [] shaderSrc[i];

        delete [] shaderSrc;
        shaderSrc = NULL;

        // check whether shader has compiled
        glGetObjectParameterivARB(_vertexShaderObj, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
        if (!compiled)
        {
            std::cout << vertexShaderNames[countVS-1] << std::endl;
            printInfoLog(std::cerr, _vertexShaderObj);
            delete [] definesStr;
            return false;
        }

        // attach vertex shader
        glAttachObjectARB(_programObj, _vertexShaderObj);

        _vertShaderAttached = true;
    }

    // load fragment shader source files
    if (countFS > 0)
    {
		numShaders = countFS + 2;
		shaderSrc = new char*[numShaders];

		shaderSrc[0] = new char[strlen(versionStr) + 1];
		strcpy(shaderSrc[0], versionStr);

		shaderSrc[1] = new char[strlen(definesStr) + 1];
		strcpy(shaderSrc[1], definesStr);
        
        for (int i=0; i<countFS; ++i)
        {
            //shaderSrc[2*i+1] = new char[128];
            //snprintf(shaderSrc[2*i+1], 128, "#line 0 %d\n", i);
            shaderSrc[i+2] = loadSource(fragmentShaderNames[i]);
        }

        // load source into shader object
        glShaderSourceARB(_fragmentShaderObj, numShaders, 
                          (const GLcharARB**) shaderSrc, NULL);

        // compile shader
        glCompileShaderARB(_fragmentShaderObj);

        for (int i=0; i<numShaders; ++i)
            delete [] shaderSrc[i];

        delete [] shaderSrc;
        shaderSrc = NULL;

        // check whether shader has compiled
        glGetObjectParameterivARB(_fragmentShaderObj, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
        if (!compiled)
        {
            std::cout << fragmentShaderNames[countFS-1] << std::endl;
            printInfoLog(std::cerr, _fragmentShaderObj);
            delete [] definesStr;
            return false;
        }

        // attach fragment shader
        glAttachObjectARB(_programObj, _fragmentShaderObj);

        _fragShaderAttached = true;
    }
    delete [] definesStr;

    // link program object
    glLinkProgramARB(_programObj);

    // check whether shader has compiled
    glGetObjectParameterivARB(_programObj, GL_OBJECT_LINK_STATUS_ARB, &compiled);
    if (compiled == GL_FALSE)
    {
        printInfoLog(std::cerr, _programObj);
        std::cerr << "GLSLShader:  Could not link shader." << std::endl;
        return false;
    }
    CHECK_FOR_OGL_ERROR();

    //glUseProgramObjectARB(_programObj);
    
    return true;
}


void GLSLShader::printInfoLog(FILE *file, GLhandleARB object)
{
    int maxLength = 0;
    char *infoLog = NULL;

    glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, 
        &maxLength);

    infoLog = new char[maxLength];
    glGetInfoLogARB(object, maxLength, &maxLength, infoLog);
    fprintf(file, "%s\n", infoLog);
    //std::err << infoLog << std::endl;

    delete [] infoLog;
}


void GLSLShader::printInfoLog(std::ostream &stream, GLhandleARB object)
{
    int maxLength = 0;
    char *infoLog = NULL;

    glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, 
        &maxLength);

    infoLog = new char[maxLength];
    glGetInfoLogARB(object, maxLength, &maxLength, infoLog);
    stream << infoLog << std::endl;

    delete [] infoLog;
}


char* GLSLShader::loadSource(char *fileName, int *size)
{
    int len;
    std::ifstream srcFile;
    char *shaderSrc = NULL;
    
    if (!fileName)
        return false;

    srcFile.open(fileName, std::ios::in | std::ios::binary);

    if (!srcFile.is_open())
    {
        std::cerr << "GLSLShader:  Could not open shader source \"" 
            << fileName << "\"." << std::endl;
        return NULL;
    }

    // get length of file:
    srcFile.seekg(0, std::ios::end);
    len = (int) srcFile.tellg();
    srcFile.seekg(0, std::ios::beg);

    shaderSrc = new char[len+1];

    // read shader out of file
    srcFile.read(shaderSrc, len);
    srcFile.close();

    shaderSrc[len] = '\0';

	//std::stringstream continut;
	//continut << srcFile.rdbuf();
	//std::string srcf = continut.str();

    if (size)
        *size = len + 1;

    return shaderSrc;
}


// -------------------------------------------------------------------


GLSLParamsLIC::GLSLParamsLIC(void) : viewport(-1),texMax(-1),scaleVol(-1),
                                     scaleVolInv(-1),stepSize(-1),gradient(-1),
                                     licParams(-1),licKernel(-1),numIterations(-1),
                                     alphaCorrection(-1),volumeSampler(-1),scalarSampler(-1),
									 licVolumeSampler(-1), licVolumeSamplerOld(-1),
                                     noiseSampler(-1),mcOffsetSampler(-1),
                                     transferRGBASampler(-1),
                                     transferAlphaOpacSampler(-1),
                                     licKernelSampler(-1),malloDiffSampler(-1),
                                     malloSpecSampler(-1),zoecklerSampler(-1),
                                     imageFBOSampler(-1),
									 interpSize(-1),interpStep(-1)
{
}


void GLSLParamsLIC::getMemoryLocations(GLhandleARB programObj, bool printList)
{
    int numUniforms;
    char buf[50];
    GLenum arrayType;
    int arraySize;
    int len;

    viewport = -1;
    texMax = -1;
    scaleVol = -1;
    scaleVolInv = -1;
    stepSize = -1;
    gradient = -1;
    licParams = -1;
    licKernel = -1;
    numIterations = -1;
    alphaCorrection = -1;

    volumeSampler = -1;
	scalarSampler = -1;
    noiseSampler = -1;
    mcOffsetSampler = -1;
    transferRGBASampler = -1;
    transferAlphaOpacSampler = -1;
    licKernelSampler = -1;
    malloDiffSampler = -1;
    malloSpecSampler = -1;
    zoecklerSampler = -1;
	licVolumeSampler = -1;
	licVolumeSamplerOld = -1;

    imageFBOSampler = -1;

	interpSize = -1;
	interpStep = -1;

    glGetObjectParameterivARB(programObj, 
        GL_OBJECT_ACTIVE_UNIFORMS_ARB, &numUniforms);

    for (int i=0; i<numUniforms; ++i)
    {
        glGetActiveUniformARB(programObj, i, 50, &len, 
            &arraySize, &arrayType, buf);

        if (printList)
        {
            std::cout << "Uniform " << std::setw(2) << i << "    " << buf << " (" << arraySize << ")" << std::endl;
        }

        // skip any gl related uniforms
        if (len > 2)
            if ((buf[0] == 'g') && (buf[1] == 'l') && (buf[2] == '_'))
                continue;

        if (strcmp(buf, "viewport") == 0)
        {
            viewport = i;
        }
        else if (strcmp(buf, "texMax") == 0)
        {
            texMax = i;
        }
        else if (strcmp(buf, "scaleVol") == 0)
        {
            scaleVol = i;
        }
        else if (strcmp(buf, "scaleVolInv") == 0)
        {
            scaleVolInv = i;
        }
        else if (strcmp(buf, "stepSize") == 0)
        {
            stepSize = i;
        }
        else if (strcmp(buf, "gradient") == 0)
        {
            gradient = i;
        }
        else if (strcmp(buf, "licParams") == 0)
        {
            licParams = i;
        }
        else if (strcmp(buf, "licKernel") == 0)
        {
            licKernel = i;
        }
        else if (strcmp(buf, "numIterations") == 0)
        {
            numIterations = i;
        }
        else if (strcmp(buf, "alphaCorrection") == 0)
        {
            alphaCorrection = i;
        }
        else if (strcmp(buf, "volumeSampler") == 0)
        {
            volumeSampler = i;
        }
		else if (strcmp(buf, "licVolumeSampler") == 0)
		{
			licVolumeSampler = i;
		}
		else if (strcmp(buf, "licVolumeSamplerOld") == 0)
		{
			licVolumeSamplerOld = i;
		}
		else if (strcmp(buf, "interpSize") == 0)
		{
			interpSize = i;
		}
		else if (strcmp(buf, "interpStep") == 0)
		{
			interpStep = i;
		}
		else if (strcmp(buf, "scalarSampler") == 0)
		{
			scalarSampler = i;
		}
        else if (strcmp(buf, "noiseSampler") == 0)
        {
            noiseSampler = i;
        }
        else if (strcmp(buf, "mcOffsetSampler") == 0)
        {
            mcOffsetSampler = i;
        }
        else if (strcmp(buf, "transferRGBASampler") == 0)
        {
            transferRGBASampler = i;
        }
        else if (strcmp(buf, "transferAlphaOpacSampler") == 0)
        {
            transferAlphaOpacSampler = i;
        }
        else if (strcmp(buf, "licKernelSampler") == 0)
        {
            licKernelSampler = i;
        }
        else if (strcmp(buf, "malloDiffSampler") == 0)
        {
            malloDiffSampler = i;
        }
        else if (strcmp(buf, "malloSpecSampler") == 0)
        {
            malloSpecSampler = i;
        }
        else if (strcmp(buf, "zoecklerSampler") == 0)
        {
            zoecklerSampler = i;
        }
        else if (strcmp(buf, "imageFBOSampler") == 0)
        {
            imageFBOSampler = i;
        }
        /*
        else if (strcmp(buf, "") == 0)
        {
        = i;
        }
        */
        else
        {
            std::cout << "unhandled uniform:   " << buf << std::endl;
        }
    }
    CHECK_FOR_OGL_ERROR();
}


// -------------------------------------------------------------------


GLSLParamsBackground::GLSLParamsBackground(void) : viewport(-1),bgColor(-1),
                                                   imageFBOSampler(-1)
{
}


void GLSLParamsBackground::getMemoryLocations(GLhandleARB programObj, bool printList)
{
    int numUniforms;
    char buf[50];
    GLenum arrayType;
    int arraySize;
    int len;

    viewport = -1;
    bgColor = -1;
    imageFBOSampler = -1;

    glGetObjectParameterivARB(programObj, 
        GL_OBJECT_ACTIVE_UNIFORMS_ARB, &numUniforms);

    for (int i=0; i<numUniforms; ++i)
    {
        glGetActiveUniformARB(programObj, i, 50, &len, 
            &arraySize, &arrayType, buf);

        if (printList)
        {
            std::cout << "Uniform " << std::setw(2) << i << "    " << buf << " (" 
                << arraySize << ")" << std::endl;
        }

        // skip any gl related uniforms
        if (len > 2)
            if ((buf[0] == 'g') && (buf[1] == 'l') && (buf[2] == '_'))
                continue;

        if (strcmp(buf, "viewport") == 0)
        {
            viewport = i;
        }
        else if (strcmp(buf, "bgColor") == 0)
        {
            bgColor = i;
        }
        else if (strcmp(buf, "imageFBOSampler") == 0)
        {
            imageFBOSampler = i;
        }
        else
        {
            std::cout << "unhandled uniform:   " << buf << std::endl;
        }
    }
    CHECK_FOR_OGL_ERROR();
}


#if 0

ShaderProgram *loadShaderProgram(const char *filename, const char *name)
{
   ShaderProgram *prog = NULL;
   char *code, *code_h, *code_tmp, *filename_tmp, fn_h[200];
   int majorType, minorType;

   char *pos_begin, *pos_end, *pos_fn_begin;
   char *includedFiles[100];
   int numIncludedFiles, alreadyIncluded, i;
   int filename_length, code_size, code_size_h;

   int   defineMakroLength;
   char* defineMakroBegin;

   code_tmp = sourceLoader(filename, name, &code_size);

   if (!code_tmp)
       return NULL;

   /*
      name string may contain a section following  "##", which is
      inserted at the beginning of the shader source code, and does
      not contribute to the shadername
    */
   defineMakroBegin  = strstr(name, "##");
   defineMakroLength = 0;

   if (defineMakroBegin != NULL)
   {
       defineMakroBegin+=2;
       defineMakroLength = (int) strlen(defineMakroBegin);

       if (! (code = (char *)malloc((code_size+defineMakroLength+1+40)*sizeof(char))))
       {
           fprintf(stderr, "not enough memory for program\n");
           exit(1);
       }

       code_size += defineMakroLength+1;

/* DIRTY HACK !! */
       strcpy(code, "#version 120\n");
/* DIRTY HACK !! */
       strcat(code, defineMakroBegin);
       strcat(code, "\n");
       strcat(code, code_tmp);

       free(code_tmp);

       /* add \0 into name variable */
       *(defineMakroBegin-2) = '\0';
   }
   else
   {
       code =     code_tmp;
   }

   programType(code, &majorType, &minorType);

   /* glsl-code: handle '#include "filename"'-macros */
   if (majorType == GL_VERTEX_SHADER_ARB ||
       majorType == GL_GEOMETRY_SHADER_EXT ||
       majorType == GL_FRAGMENT_SHADER_ARB)
   {


       includedFiles[0] = (char *) filename;
       numIncludedFiles = 1;

       while ((pos_begin = strstr(code,"#include")))
       {               pos_fn_begin    = strstr(pos_begin+9,"\"");
           pos_end         = strstr(pos_fn_begin+1,"\"");
           filename_length = (int) (pos_end - pos_fn_begin)-1;

           if (filename_length > 199)
           {
               fprintf(stderr, "%s - #include directive parse error! filename to long? \n", filename);
                                      /* delete filename list */
               for (i=1; i<numIncludedFiles; i++)
                   free(includedFiles[i]);

               return NULL;
           }

           /* header filename */
           memcpy(fn_h, &pos_fn_begin[1], filename_length);
           fn_h[filename_length] = '\0';

           alreadyIncluded = 0;

           for (i=0; i<numIncludedFiles; i++)
           {                                 if (!strcmp(fn_h, includedFiles[i]))
                   alreadyIncluded = 1;
           }

           if (!alreadyIncluded)
           {
               code_h = sourceLoader(fn_h, name, &code_size_h);

               if (!code_h || numIncludedFiles==100)
               {
                   if (numIncludedFiles==100)
                       perror("include limit of 100 files exceeded!");
                   else
                       fprintf(stderr, "%s - could not open file %s\n", filename, fn_h);
                  
                   /* delete filename list */
                   for (i=1; i<numIncludedFiles; i++)
                       free(includedFiles[i]);
                   return NULL;
               }

               code_size_h = (int) strlen(code_h);

               /* buffer for included code */
               code_size = (NVIDIA_glShaderSourceARB_BUG(code_size+code_size_h + 1));
               if (! (code_tmp = (char *) malloc(code_size*sizeof(char)))) {

                   fprintf(stderr, "not enough memory for program\n");
                   exit(1);
               }

               /* update code buffer */
               memcpy(code_tmp, code, pos_begin-code);
               memcpy(code_tmp+(pos_begin-code), code_h, code_size_h);
               strcpy(code_tmp+(pos_begin-code)+code_size_h, pos_end+1);

               free(code);
               free(code_h);
               code = code_tmp;

               /* add filename to included-list */
               if (! (filename_tmp = (char *)malloc((strlen(fn_h)+1)*sizeof(char))))
               {
                   fprintf(stderr, "not enough memory for "
                       "program\n");
                   exit(1);
               }

               strcpy(filename_tmp, fn_h);
               includedFiles[numIncludedFiles] = filename_tmp;
               numIncludedFiles++;

           }
           else
           {
             /* remove include directive */
               for (i=0; i<=(pos_end - pos_begin); i++)
                   pos_begin[i] = ' ';
           }
       }                      /* delete filename list */
       for (i=1; i<numIncludedFiles; i++)
           free(includedFiles[i]);

   }


   switch (majorType) {
       case GL_FRAGMENT_PROGRAM_ARB:
       case GL_VERTEX_PROGRAM_ARB:
           if (!glh_init_extensions("GL_ARB_vertex_program "
                                    "GL_ARB_fragment_program ")) {
               fprintf(stderr, "Shader %s not supported: %s missing\n",
                       name, glh_get_unsupported_extensions());
               return NULL;
           }
           if (minorType == GL_NV_GPU_PROGRAM4 &&
               !glh_init_extensions("GL_NV_gpu_program4 ")) {
               fprintf(stderr, "Shader %s not supported: %s missing\n",
                       name, glh_get_unsupported_extensions());
               return NULL;
           }
           prog = newARBProgram(name);
           prog->type = majorType;
           if (!parseARBProgram(prog, filename, code)) {
               /*
               fprintf(stderr, "exiting due to parse error in program "
                       "'%s'\n", filename);
               */
               freeShaderProgram(&prog);
               free(code);
               return NULL;
           }
           break;
       case GL_GEOMETRY_SHADER_EXT:
           if (!glh_init_extensions("GL_EXT_geometry_shader4 ")) {
               fprintf(stderr, "Shader %s not supported: %s missing\n",
                       name, glh_get_unsupported_extensions());
               return NULL;
           }
           /* Attention: fall through case! */
       case GL_VERTEX_SHADER_ARB:
       case GL_FRAGMENT_SHADER_ARB:
           if (!glh_init_extensions("GL_ARB_shader_objects "
                                    "GL_ARB_vertex_shader "
                                    "GL_ARB_fragment_shader "
                                    "GL_ARB_shading_language_100 ")) {
               fprintf(stderr, "Shader %s not supported: %s missing\n",
                       name, glh_get_unsupported_extensions());
               return NULL;
           }
           prog = newGLSLProgram(name);
           prog->type = majorType;
           if (!parseGLSLProgram(prog, filename, code)) {
               fprintf(stderr, "exiting due to parse error in program "
                       "'%s'\n", filename);
               freeShaderProgram(&prog);
               free(code);
               return NULL;
           }
           break;
       default:
           fprintf(stderr, "can't determine program type for %s\n",
                   filename);
   }
      free(code);

   /* remove \0 from const char * name */
   if (defineMakroBegin)
   {
       *(defineMakroBegin-2) = ' ';
   }

   return prog;
}

#endif



