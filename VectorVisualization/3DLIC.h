#pragma once

#include "transform.h"
#include "camera.h"
#include "hud.h"
#include "renderer.h"
#include "transferEdit.h"
#include "fpsCounter.h"
#include "dataSet.h"
#include "parseArg.h"
#include "CPULIC.h"

HDC dc;
HGLRC glrc_main;
HGLRC glrc_load;

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
VectorDataSet vector;
NoiseDataSet noise;
Illumination illum;
LICFilter licFilter;

OpenGLHUD hud;

CPULIC *cpuLic;

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

RenderTechnique renderTechnique = VOLIC_RAYCAST;
bool animationMode = false;
MouseMode mouseMode;
LICParams licParams;
LAOParams laoParams;

void display(void);
void resize(int width, int height);
void updateHUD(bool forceUpdate = false);
void keyboard(unsigned char key, int x, int y);

//Multi-thread
std::mutex mt_load;
bool load_volume_buffer = false;
std::condition_variable cv_load;