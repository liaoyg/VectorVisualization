#include <Windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include "types.h"
#include "3DLIC.h""
#include "ogldev_util.h"
#include "ogldev_math_3d.h"

using namespace std;

GLuint VBO;
GLuint gScaleLocation;

const char* pVSFileName = "shader/shader.vs";
const char* pFSFileName = "shader/shader.fs";

void changeViewPort(int w, int h)
{
	glViewport(0, 0, w, h);
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
}

void display(void)
{
	char fpsStr[10];

	//fpsCounter.frameStart();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glClear(GL_COLOR_BUFFER_BIT);

	static float Scale = 0.0f;

	Scale += 0.001f;

	glUniform1f(gScaleLocation, sinf(Scale));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(0);


	//renderer.render(updateScene || updateSceneCont);
	//updateScene = false;
	CHECK_FOR_OGL_ERROR();

	//// render light
	//if (lightVisible)
	//	renderer.renderLight(!currentClipPlane);
	//CHECK_FOR_OGL_ERROR();

	//// draw transfer function editor
	//tfEdit.draw();

	//// show HUD
	//snprintf(fpsStr, 10, "%2.2f", fpsCounter.getFPS());
	//hud.DrawHUD(fpsStr, 420, 2);
	////hud.DrawHUD();

	CHECK_FOR_OGL_ERROR();

	glutSwapBuffers();
	//fpsCounter.frameFinished();
}

static void CreateVertexBuffer()
{
	Vector3f Vertices[3];
	Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);
	Vertices[1] = Vector3f(1.0f, -1.0f, 0.0f);
	Vertices[2] = Vector3f(0.0f, 1.0f, 0.0f);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

void resize(int width, int height)
{
	if (width < 1)
		width = 1;
	if (height < 1)
		height = 1;

	int viewport[4] = { 0, 0, width, height };

	//w = width;
	//h = height;

	glViewport(0, 0, width, height);
	CHECK_FOR_OGL_ERROR();

	//renderer.resize(width, height);
	CHECK_FOR_OGL_ERROR();

	//cam.setWindow(width, height);
	//light.setWindow(width, height);

	//for (int i = 0; i<3; ++i)
	//	clipPlanes[i].setWindow(width, height);

	//tfEdit.resize(width, height);
	//hud.SetViewport(viewport, true);
	//CHECK_FOR_OGL_ERROR();

	//aspect = (float)width / height;

	//updateScene = true;
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(1);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders()
{
	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	string vs, fs;

	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	};

	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	};

	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glUseProgram(ShaderProgram);

	gScaleLocation = glGetUniformLocation(ShaderProgram, "gScale");
	assert(gScaleLocation != 0xFFFFFFFF);
}

void initGL(void)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//renderer.init();

	//if (!hud.Init())
	//{
	//	std::cerr << "Could not initialize HUD" << std::endl;
	//	exit(-1);
	//}
	//hud.SetNumLines(2);
	////hud.SetText("Volumetric LIC (sample)");
	//updateHUD();

	CHECK_FOR_OGL_ERROR();
}

void init(void)
{
	//VolumeData *volumeData = NULL;

	//light.setDistance(1.0f);

	//renderer.setLight(&light);
	//renderer.setCamera(&cam);

	//// load data set    
	//if (!vd.loadData(arguments.getVolFileName()))
	//{
	//	std::cerr << "Could not load data ..." << std::endl;
	//	exit(1);
	//}
	//vd.getVolumeData()->data = vd.loadTimeStep(vd.getTimeStepBegin());
	//vd.createTexture("VectorData_Tex", GL_TEXTURE2_ARB, true);

	//// load noise data
	//noise.loadData(arguments.getNoiseFileName());
	//noise.enableGradient(arguments.getGradientsFlag());
	//noise.createTexture("Noise_Tex", GL_TEXTURE3_ARB);
	//std::cout << std::endl;

	//// load LIC filter kernel
	//if (!licFilter.loadData(arguments.getLicFilterFileName()))
	//{
	//	std::cerr << "could not load lic filter kernel ... using box filter"
	//		<< std::endl;
	//	licFilter.createBoxFilter();
	//}
	//licFilter.createTexture("LIC_kernel_Tex", GL_TEXTURE5_ARB);

	//// create illumination textures for Zoeckler and Mallo
	////illum.setDebugMode(true);
	//illum.createIllumTextures(true, true, true);
	//glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, illum.getSpecularExp());
	//std::cout << std::endl;

	//if (arguments.getTfFileName())
	//{
	//	if (!tfEdit.loadTF(arguments.getTfFileName()))
	//	{
	//		std::cerr << "could not load transfer function"
	//			<< std::endl;
	//	}
	//}
	//updateScene = tfEdit.isSceneUpdateNeeded();
	//tfEdit.updateTextures();
	//tfEdit.computeHistogram(vd.getVolumeData());
	//CHECK_FOR_OGL_ERROR();

	//if (!hud.Init())
	//{
	//	std::cerr << "Could not initialize HUD" << std::endl;
	//	exit(-1);
	//}
	//hud.SetNumLines(2);
	////hud.SetText("Volumetric LIC (sample)");
	//updateHUD();
	//CHECK_FOR_OGL_ERROR();
	////std::cout << std::endl;

	//clipPlanes[0].rotate(Quaternion_fromAngleAxis(static_cast<float>(M_PI / 2.0), Vector3_new(0.0f, 1.0f, 0.0f)));
	//clipPlanes[1].rotate(Quaternion_fromAngleAxis(static_cast<float>(M_PI / 2.0), Vector3_new(-1.0f, 0.0f, 0.0f)));

	//volumeData = vd.getVolumeData();
	//currentClipPlane = NULL;
	//for (int i = 0; i<3; ++i)
	//{
	//	clipPlanes[i].setPlaneId(GL_CLIP_PLANE0 + i);

	//	clipPlanes[i].setBoundingBox(
	//		-volumeData->extent[0] / 2.0f,
	//		-volumeData->extent[1] / 2.0f,
	//		-volumeData->extent[2] / 2.0f,
	//		volumeData->extent[0] / 2.0f,
	//		volumeData->extent[1] / 2.0f,
	//		volumeData->extent[2] / 2.0f);
	//}

	//renderer.setClipPlanes(clipPlanes, 3);
	//renderer.setTechnique(renderTechnique);
	//renderer.setVolumeData(vd.getVolumeData());
	//renderer.setLICFilter(&licFilter);

	//renderer.setDataTex(vd.getTextureRef());
	//renderer.setNoiseTex(noise.getTextureRef());
	//renderer.setTFrgbTex(tfEdit.getTextureRGB());
	//renderer.setTFalphaOpacTex(tfEdit.getTextureAlphaOpac());
	//renderer.setIllumZoecklerTex(illum.getTexZoeckler());
	//renderer.setIllumMalloDiffTex(illum.getTexMalloDiffuse());
	//renderer.setIllumMalloSpecTex(illum.getTexMalloSpecular());

	//renderer.setLICParams(&licParams);

	//renderer.updateLightPos();
	//renderer.updateSlices();
}

int main(int argc, char* argv[]) {

	// Initialize GLUT
	glutInit(&argc, argv);
	// Set up some memory buffers for our display
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA);
	// Set the window size
	glutInitWindowSize(800, 600);
	// Create the window with the title "Hello,GL"
	glutCreateWindow("VecVis");
	// Bind the two functions (above) to respond when necessary
	glutReshapeFunc(changeViewPort);
	glutDisplayFunc(display);
	glutIdleFunc(display);
	// Very important!  This initializes the entry points in the OpenGL driver so we can 
	// call all the functions in the API.
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW error");
		return 1;
	}

	initGL();
	init();


	CreateVertexBuffer();

	CompileShaders();

	glutMainLoop();
	return 0;
}