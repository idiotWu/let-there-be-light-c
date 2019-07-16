#ifndef EXPR_FX_H
#define EXPR_FX_H

#include "render/engine.h"
#include "scene/scene.h"

void fxFadeIn(double duration, Scene* toScene);
void fxFadeOut(double duration, Scene* toScene);

void fxExplode(int spriteRow, double x, double y, double size);
void fxExplodeAtCell(int spriteRow, int x, int y);

void fxFlood(int x, int y);

#endif
