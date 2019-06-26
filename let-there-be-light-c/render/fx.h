#ifndef EXPR_FX_H
#define EXPR_FX_H

#include "engine.h"

Animation* fxDoorClose(double duration);
Animation* fxDoorOpen(double duration);
Animation* fxExplode(int spriteRow, int x, int y);

void fxFlood(int x, int y);

#endif
