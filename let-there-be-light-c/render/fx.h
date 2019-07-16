#ifndef EXPR_FX_H
#define EXPR_FX_H

#include "engine.h"
#include "scene/scene.h"

void fxFadeIn(double duration, Scene* toScene);
void fxFadeOut(double duration, Scene* toScene);

void fxExplode(int spriteRow, int x, int y);

void fxFlood(int x, int y);

#endif
