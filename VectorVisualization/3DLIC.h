#pragma once

#include "types.h"

int mousePosOld[2];

// TODO: delete global variables
float aspect = 1.0f;
bool screenshot = false;
int w = -1;
int h = -1;
bool wire = false;

bool updateScene = true;
bool updateSceneCont = false;
bool requestHighRes = false;
bool sceneMoved = false;
bool mouseBtnDown = false;
bool lightVisible = false;

bool useIdle = true;

double lowResTimer = 0.0;

RenderTechnique renderTechnique = VOLIC_VOLUME;
MouseMode mouseMode;
LICParams licParams;


void display(void);
void resize(int width, int height);