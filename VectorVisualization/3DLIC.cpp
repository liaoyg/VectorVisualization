#include <Windows.h>

#define _USE_MATH_DEFINES
#define GLH_EXT_SINGLE_FILE

#include <GL\glew.h>
#include <GL\freeglut.h>

#include <math.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>

#include "texture.h"
#include "illumination.h"
#include "hud.h"
#include "transform.h"
#include "camera.h"
#include "parseArg.h"
#include "imageUtils.h"
#include "dataSet.h"
#include "types.h"
#include "timer.h"
#include "parseArg.h"
#include "3DLIC.h"


void displayTest(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(35.0, aspect, 0.1, 10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -5.0f);

	glBegin(GL_TRIANGLE_FAN);
	{
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

		glVertex3i(-1, -1, 0);
		glVertex3i(1, -1, 0);
		glVertex3i(0, 1, 0);
	}
	glEnd();

	if (screenshot)
	{
		Image img;

		img.imgData = new unsigned char[3 * w*h];
		img.width = w;
		img.height = h;
		img.channel = 3;

		glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, img.imgData);
		CHECK_FOR_OGL_ERROR();

		if (!pngWrite("test.png", &img, true))
			std::cerr << "error writing png" << std::endl;
		if (!ppmWrite("test.ppm", &img))
			std::cerr << "error writing ppm" << std::endl;

		delete[] img.imgData;
		img.imgData = NULL;

		if (!pngRead("test.png", &img))
			std::cerr << "error reading png" << std::endl;
		if (!pngWrite("test2.png", &img))
			std::cerr << "error rewriting png" << std::endl;
		delete[] img.imgData;
		img.imgData = NULL;


		if (!ppmRead("test.ppm", &img))
			std::cerr << "error reading ppm" << std::endl;
		if (!ppmWrite("test2.ppm", &img))
			std::cerr << "error rewriting ppm" << std::endl;

		std::cout << "done" << std::endl;

		screenshot = false;
	}

	glutSwapBuffers();
}


void display(void)
{
	char fpsStr[10];

	fpsCounter.frameStart();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//double timetest = timer();

	renderer.render(updateScene || updateSceneCont);

	//std::cout << "cost for render" << timetest - timer() << std::endl;
	//updateScene = true;
	CHECK_FOR_OGL_ERROR();

	// render light
	if (lightVisible)
		renderer.renderLight(!currentClipPlane);
	CHECK_FOR_OGL_ERROR();

	// draw transfer function editor
	tfEdit.draw();

	// show HUD
	snprintf(fpsStr, 10, "%2.2f", fpsCounter.getFPS());
	hud.DrawHUD(fpsStr, 420, 2);
	//hud.DrawHUD();

	CHECK_FOR_OGL_ERROR();

	glutSwapBuffers();
	fpsCounter.frameFinished();
}


void idle(void)
{
	//Move volume data to next time step
	int idx = vd.getCurTimeStep();
	//std::cout << "current animation step: " << idx << std::endl;
	//vd.getVolumeData()->data = vd.getVolumeData()->dataSets[idx];
	//vd.getVolumeData()->newData = vd.getVolumeData()->dataSets[vd.NextTimeStep()];

	//vd.createTexture("VectorData_Tex", GL_TEXTURE2_ARB, true);
	vd.createTextureIterp("VectorData_Tex", GL_TEXTURE2_ARB, true);
	vd.checkInterpolateStage();

	//renderer.setDataTex(vd.getTextureSetRef(idx));

	//Update Render Animation source
	renderer.setVolumeData(vd.getVolumeData());

	// check whether to change to high res rendering
	if (requestHighRes && renderer.isLowResEnabled())
	{
		double currentTime = timer();
		if (currentTime - lowResTimer > LOW_RES_TIMER_DELAY*1000.0)
		{
			renderer.enableLowRes(false);
			requestHighRes = false;
			updateScene = true;

			updateHUD();
			/*
			if (g.technique == VOLIC_SLICING)
			updateSlicing();
			*/
		}
	}

	if (renderTechnique == VOLIC_SLICING)
		renderer.updateSlices();

	glutPostRedisplay();
}


void resize(int width, int height)
{
	if (width < 1)
		width = 1;
	if (height < 1)
		height = 1;

	int viewport[4] = { 0, 0, width, height };

	w = width;
	h = height;

	glViewport(0, 0, width, height);
	CHECK_FOR_OGL_ERROR();

	renderer.resize(width, height);
	CHECK_FOR_OGL_ERROR();

	cam.setWindow(width, height);
	light.setWindow(width, height);

	for (int i = 0; i<3; ++i)
		clipPlanes[i].setWindow(width, height);

	tfEdit.resize(width, height);
	hud.SetViewport(viewport, true);
	CHECK_FOR_OGL_ERROR();

	aspect = (float)width / height;

	updateScene = true;
}


void updateHUD(bool forceUpdate)
{
	char buf[1024];
	char technique[30];

	switch (renderTechnique)
	{
	case VOLIC_VOLUME:
		snprintf(technique, 30, "Volume     ");
		break;
	case VOLIC_RAYCAST:
		snprintf(technique, 30, "Raycast    ");
		break;
	case VOLIC_SLICING:
		snprintf(technique, 30, "Slicing    ");
		break;
	}

	snprintf(buf, 1024, "%s%s   Samp. Dist: %.6f   LIC Params: %.4f  %d/%d\n"
		"Gradient Scale: %.1f   Freqency Scale: %.1f   Illum Scale: %.2f  %s%s",
		technique, (renderer.isFBOenabled() ? " (FBO)" : ""),
		licParams.stepSizeVol, licParams.stepSizeLIC,
		licParams.stepsForward, licParams.stepsBackward,
		licParams.gradientScale, licParams.freqScale,
		licParams.illumScale,
		(updateSceneCont ? "cont" : ""),
		(renderer.isLowResEnabled() ? " lowRes" : ""));

	hud.SetText(buf, forceUpdate);
}


void keyboard(unsigned char key, int x, int y)
{
	bool handled;

	handled = tfEdit.keyboardTE(key, x, y);
	if (handled)
	{
		updateScene = tfEdit.isSceneUpdateNeeded();
		return;
	}

	switch (key)
	{
	case 'q':
	case 27:
		exit(1);
		break;
	case '0':
		screenshot = true;
		renderer.screenshot();
		updateScene = true;
		break;
	case 'H':
		hud.SetVisible(!hud.IsVisible());
		break;
	case 'r':
		renderer.loadGLSLShader();
		updateScene = true;
		break;
	case 'w':
		wire = !wire;
		renderer.setWireframe(wire);
		break;
	case 't':
		tfEdit.toggleVisibility();
		break;
	case 'F':
		renderer.enableFBO(!renderer.isFBOenabled());
		updateHUD();
		updateScene = true;
		break;
	case 'p':
		tfEdit.updateTextures();
		updateScene = true;
		break;
	case 'L': // lowres rendering
		renderer.enableLowRes(!renderer.isLowResEnabled());
		requestHighRes = false;
		updateScene = true;
		updateHUD();
		break;
	case 'I': // switch idle redrawing
		useIdle = !useIdle;
		if (useIdle)
			glutIdleFunc(idle);
		else
			glutIdleFunc(NULL);
		updateScene = true;
		break;

		// lic params
	case '[': // stepsize/2
		licParams.stepSizeVol /= 2.0f;
		if (licParams.stepSizeVol < MIN_STEPSIZE)
			licParams.stepSizeVol = MIN_STEPSIZE;
		updateHUD();
		updateScene = true;
		break;
	case ']': // 2*stepsize
		licParams.stepSizeVol *= 2.0f;
		if (licParams.stepSizeVol > MAX_STEPSIZE)
			licParams.stepSizeVol = MAX_STEPSIZE;
		updateHUD();
		updateScene = true;
		break;
	case 's': // LIC steps forward
		++licParams.stepsForward;
		updateHUD();
		updateScene = true;
		break;
	case 'x':
		--licParams.stepsForward;
		if (licParams.stepsForward < 1)
			licParams.stepsForward = 1;
		updateHUD();
		updateScene = true;
		break;
	case 'S': // LIC steps backward
		++licParams.stepsBackward;
		updateHUD();
		updateScene = true;
		break;
	case 'X':
		--licParams.stepsBackward;
		if (licParams.stepsBackward < 1)
			licParams.stepsBackward = 1;
		updateHUD();
		updateScene = true;
		break;
	case 'a': // LIC stepSize
		licParams.stepSizeLIC *= 2.0f; // += 0.05f
		updateHUD();
		updateScene = true;
		break;
	case 'z':
		licParams.stepSizeLIC *= 0.5f; // -= 0.05f
		if (licParams.stepSizeLIC < 0.0005f)
			licParams.stepSizeLIC = 0.0005f;
		updateHUD(); updateScene = true;
		break;
	case 'h': // frequency scaling
		licParams.freqScale += 0.2f; //0.5f
		updateHUD();
		updateScene = true;
		break;
	case 'n':
		licParams.freqScale -= 0.2f; //0.5f
		if (licParams.freqScale < 0.5f)
			licParams.freqScale = 0.5f;
		updateHUD();
		updateScene = true;
		break;
	case 'j': // illumination scaling
		licParams.illumScale += 0.05f;
		updateHUD();
		updateScene = true;
		break;
	case 'm':
		licParams.illumScale -= 0.05f;
		if (licParams.illumScale < 0.05f)
			licParams.illumScale = 0.05f;
		updateHUD();
		updateScene = true;
		break;
	case 'g': // gradient scale
		licParams.gradientScale += 0.2f;
		updateHUD();
		updateScene = true;
		break;
	case 'b':
		licParams.gradientScale -= 0.2f;
		if (licParams.gradientScale < 0.2f)
			licParams.gradientScale = 0.2f;
		updateHUD();
		updateScene = true;
		break;

		// clip planes
	case '1':
	case '2':
	case '3':
		switchClipPlane(&currentClipPlane, &clipPlanes[key - '1']);
		updateScene = true;
		break;
	case '4':
		switchClipPlane(&currentClipPlane, NULL);
		updateScene = true;
		break;
	case '5':
		lightVisible = !lightVisible;
		break;

	case '7':
		renderer.loadGLSLShader("#define ILLUM_ZOECKLER");
		updateScene = true;
		break;
	case '8':
		renderer.loadGLSLShader("#define ILLUM_MALLO");
		updateScene = true;
		break;
	case '9':
		renderer.loadGLSLShader("#define ILLUM_GRADIENT");
		updateScene = true;
		break;
		
	case '6':
		renderer.loadGLSLShader("#define SPEED_OF_FLOW");
		updateScene = true;
		break;
		

	case ' ':
		updateSceneCont = !updateSceneCont;
		renderer.enableFrameStore(!updateSceneCont);
		fpsCounter.reset();
		updateScene = true;
		updateHUD();
		break;
	default:
		break;
	}

	if (updateScene && renderTechnique == VOLIC_SLICING)
		renderer.updateSlices();
}


void keyboardSpecial(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		renderTechnique = VOLIC_VOLUME;
		break;
	case GLUT_KEY_F2:
		renderTechnique = VOLIC_RAYCAST;
		break;
	case GLUT_KEY_F3:
		renderTechnique = VOLIC_SLICING;
		renderer.updateSlices();
		break;
	}
	renderer.setTechnique(renderTechnique);
	updateHUD();
	updateScene = true;
}


void mouseInteract(int button, int state, int x, int y)
{
	int modifier = glutGetModifiers();
	bool lockedMotion = (modifier & GLUT_ACTIVE_SHIFT);

	mousePosOld[0] = x;
	mousePosOld[1] = y;

	if (state == GLUT_DOWN)
	{
		sceneMoved = false;
		mouseBtnDown = true;
	}

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (modifier & GLUT_ACTIVE_CTRL) // clip planes and light
		{
			if (currentClipPlane)
			{
				currentClipPlane->setLock(lockedMotion);
				mouseMode = VOLIC_MOUSE_ROTATE_CLIP;
			}
			else
			{
				light.setLock(lockedMotion);
				mouseMode = VOLIC_MOUSE_ROTATE_LIGHT;
			}
		}
		else // move camera
		{
			cam.setLock(lockedMotion);
			mouseMode = VOLIC_MOUSE_ROTATE;
		}
		updateScene = true;
		break;
	case GLUT_MIDDLE_BUTTON:
		mouseMode = VOLIC_MOUSE_TRANSLATE;
		updateScene = true;
		break;
	case GLUT_RIGHT_BUTTON:
		if (modifier & GLUT_ACTIVE_CTRL) // clip planes and light
		{
			if (currentClipPlane)
				mouseMode = VOLIC_MOUSE_TRANSLATE_CLIP;
			else
				mouseMode = VOLIC_MOUSE_TRANSLATE_LIGHT;
		}
		else
		{
			mouseMode = VOLIC_MOUSE_DOLLY;
		}
		updateScene = true;
		break;
	default:
		break;
	}
}


void mouseMotionInteract(int x, int y)
{
	Quaternion q_cam = cam.getQuaternion();

	switch (mouseMode)
	{
		// camera
	case VOLIC_MOUSE_ROTATE:
		cam.rotate(x, y, mousePosOld[0], mousePosOld[1]);
		break;
	case VOLIC_MOUSE_TRANSLATE:
		cam.translate(x, y, mousePosOld[0], mousePosOld[1]);
		break;
	case VOLIC_MOUSE_DOLLY:
		cam.dolly(x, y, mousePosOld[0], mousePosOld[1]);
		break;

		// light
	case VOLIC_MOUSE_ROTATE_LIGHT:
		light.rotate(x, y, mousePosOld[0], mousePosOld[1]);
		break;
	case VOLIC_MOUSE_TRANSLATE_LIGHT:
		light += 0.5f * MOUSE_SCALE * (y - mousePosOld[1]) / static_cast<float>(w);
		if (light.getDistance() < 1.0e-6f)
			light.setDistance(1.0e-6f);
		break;

		// clip planes
	case VOLIC_MOUSE_ROTATE_CLIP:
		if (currentClipPlane)
			currentClipPlane->rotate(x, y, mousePosOld[0], mousePosOld[1], q_cam);
		break;
	case VOLIC_MOUSE_TRANSLATE_CLIP:
		if (currentClipPlane)
			*currentClipPlane += MOUSE_SCALE * (mousePosOld[1] - y) / static_cast<float>(w);
		break;
	}

	mousePosOld[0] = x;
	mousePosOld[1] = y;

	if (mouseBtnDown)
	{
		// change to low res mode immediately
		//renderer.enableLowRes(true);
		//requestHighRes = false;
		mouseBtnDown = false;

		updateHUD();
	}


	// TODO: fbo tex and update slicing ;-)
	updateScene = true;
	sceneMoved = true;

	renderer.updateLightPos();
}


void mouse(int button, int state, int x, int y)
{
	bool handled = false;

	handled = tfEdit.mouseTE(x, y);

	if (!handled)
	{
		mouseInteract(button, state, x, y);
		if (updateScene && renderTechnique == VOLIC_SLICING)
			renderer.updateSlices();
	}

	if (state == GLUT_UP)
	{
		// TODO: change to high res mode after X msec
		// store current timestamp. Idle function has to do the rest

		lowResTimer = timer();
		if (renderer.isLowResEnabled() && sceneMoved)
		{
			// only do it when object has been moved
			requestHighRes = true;
			sceneMoved = false;
		}
	}
}


void motion(int x, int y)
{
	bool handled = false;

	handled = tfEdit.motionTE(x, y);

	if (!handled)
	{
		mouseMotionInteract(x, y);

		if (updateScene && renderTechnique == VOLIC_SLICING)
			renderer.updateSlices();
	}
}


void initGL(void)
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

	renderer.init();

	if (!hud.Init())
	{
		std::cerr << "Could not initialize HUD" << std::endl;
		exit(-1);
	}
	hud.SetNumLines(2);
	//hud.SetText("Volumetric LIC (sample)");
	updateHUD();

	CHECK_FOR_OGL_ERROR();
}


void init(void)
{
	VolumeData *volumeData = NULL;

	light.setDistance(1.0f);

	renderer.setLight(&light);
	renderer.setCamera(&cam);

	// load data set    
	if (!vd.loadData(arguments.getVolFileName()))
	{
		std::cerr << "Could not load data ..." << std::endl;
		exit(1);
	}
	for (int i = vd.getTimeStepBegin(); i <= vd.getTimeStepEnd(); i++)
	{
		void* datap = vd.loadTimeStep(vd.getCurTimeStep());
		vd.getVolumeData()->dataSets.push_back(datap);
		vd.getNextTimeStep();
	}
	
	vd.getVolumeData()->data = vd.getVolumeData()->dataSets[vd.getCurTimeStep()];
	vd.getVolumeData()->newData = vd.getVolumeData()->dataSets[vd.NextTimeStep()];
	//vd.createTextures("VectorData_Tex", vd.getVolumeData()->dataSets.size(), GL_TEXTURE2_ARB, true);
	vd.setInterpolateSize(100);
	vd.createTexture("VectorData_Tex", GL_TEXTURE2_ARB, true);

	// load noise data
	noise.loadData(arguments.getNoiseFileName());
	noise.enableGradient(arguments.getGradientsFlag());
	noise.createTexture("Noise_Tex", GL_TEXTURE3_ARB);
	std::cout << std::endl;

	// load secondary scalar volume data
	if (!scalar.loadData("..\\data\\outputraw\\out_64_0_temperature.dat"))
	{
		std::cerr << "Could not load data ..." << std::endl;
		exit(1);
	}
	scalar.createTexture("Scalar_Tex", GL_TEXTURE4_ARB);

	// load LIC filter kernel
	if (!licFilter.loadData(arguments.getLicFilterFileName()))
	{
		std::cerr << "could not load lic filter kernel ... using box filter"
			<< std::endl;
		licFilter.createBoxFilter();
	}
	licFilter.createTexture("LIC_kernel_Tex", GL_TEXTURE5_ARB);

	// create illumination textures for Zoeckler and Mallo
	//illum.setDebugMode(true);
	illum.createIllumTextures(true, true, true);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, illum.getSpecularExp());
	std::cout << std::endl;

	if (arguments.getTfFileName())
	{
		if (!tfEdit.loadTF(arguments.getTfFileName()))
		{
			std::cerr << "could not load transfer function"
				<< std::endl;
		}
	}
	updateScene = tfEdit.isSceneUpdateNeeded();
	tfEdit.updateTextures();
	tfEdit.computeHistogram(vd.getVolumeData());
	CHECK_FOR_OGL_ERROR();

	if (!hud.Init())
	{
		std::cerr << "Could not initialize HUD" << std::endl;
		exit(-1);
	}
	hud.SetNumLines(2);
	//hud.SetText("Volumetric LIC (sample)");
	updateHUD();
	CHECK_FOR_OGL_ERROR();
	//std::cout << std::endl;

	clipPlanes[0].rotate(Quaternion_fromAngleAxis(static_cast<float>(M_PI / 2.0), Vector3_new(0.0f, 1.0f, 0.0f)));
	clipPlanes[1].rotate(Quaternion_fromAngleAxis(static_cast<float>(M_PI / 2.0), Vector3_new(-1.0f, 0.0f, 0.0f)));

	volumeData = vd.getVolumeData();
	currentClipPlane = NULL;
	for (int i = 0; i<3; ++i)
	{
		clipPlanes[i].setPlaneId(GL_CLIP_PLANE0 + i);

		clipPlanes[i].setBoundingBox(
			-volumeData->extent[0] / 2.0f,
			-volumeData->extent[1] / 2.0f,
			-volumeData->extent[2] / 2.0f,
			volumeData->extent[0] / 2.0f,
			volumeData->extent[1] / 2.0f,
			volumeData->extent[2] / 2.0f);
	}

	renderer.setClipPlanes(clipPlanes, 3);
	renderer.setTechnique(renderTechnique);
	renderer.setVolumeData(vd.getVolumeData());
	renderer.setLICFilter(&licFilter);

	renderer.setDataTex(vd.getTextureRef());
	renderer.setScalarTex(scalar.getTextureRef());
	renderer.setNoiseTex(noise.getTextureRef());
	renderer.setTFrgbTex(tfEdit.getTextureRGB());
	renderer.setTFalphaOpacTex(tfEdit.getTextureAlphaOpac());
	renderer.setIllumZoecklerTex(illum.getTexZoeckler());
	renderer.setIllumMalloDiffTex(illum.getTexMalloDiffuse());
	renderer.setIllumMalloSpecTex(illum.getTexMalloSpecular());

	renderer.setLICParams(&licParams);

	renderer.updateLightPos();
	renderer.updateSlices();
}


int main(int argc, char **argv)
{
	arguments.setProgramName("volic");
	arguments.setArguments(argc, argv);

	if (!arguments.parse())
	{
		arguments.printUsage();
		exit(1);
	}

	glutInit(&argc, argv);
	glutInitWindowPosition(390, 20);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_ALPHA);
	glutCreateWindow("voLIC V2 - GPU-based volumetric LIC renderer");

	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	glutIdleFunc(idle);

	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW error");
		return 1;
	}
	initGL();
	init();

	glutMainLoop();

	return 0;
}