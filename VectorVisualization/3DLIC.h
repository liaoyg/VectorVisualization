#pragma once

#include "transform.h"
#include "camera.h"
#include "hud.h"
#include "renderer.h"
#include "transferEdit.h"
#include "fpsCounter.h"
#include "dataSet.h"
#include "parseArg.h"

ParseArguments arguments;
Camera cam;
Renderer renderer;
TransferEdit tfEdit;
FPSCounter fpsCounter;
ClipPlane clipPlanes[3];
ClipPlane *currentClipPlane;
Transform light;

VectorDataSet vd;
VolumeDataSet scalar; // load second dimension scalar data
NoiseDataSet noise;
Illumination illum;
LICFilter licFilter;

OpenGLHUD hud;

int mousePosOld[2];

// TODO: delete global variables
float aspect = 1.0f;
bool screenshot = false;
bool recording = false;
int w = -1;
int h = -1;
bool wire = false;

bool updateScene = true;
bool updateSceneCont = false;
bool requestHighRes = false;
bool sceneMoved = false;
bool mouseBtnDown = false;
bool lightVisible = false;
bool coordinateVisible = false;

bool useIdle = true;

double lowResTimer = 0.0;

RenderTechnique renderTechnique = VOLIC_VOLUME;
bool animationMode = false;
MouseMode mouseMode;
LICParams licParams;

void display(void);
void resize(int width, int height);
void updateHUD(bool forceUpdate = false);
void keyboard(unsigned char key, int x, int y);