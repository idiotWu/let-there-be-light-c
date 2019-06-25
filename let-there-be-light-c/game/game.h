#ifndef GAME_H
#define GAME_H

#include "config.h"

#include "maze/tile.h"

void initGame(void);
void buildWorld(void);
void updateGame(void);
void expandVision(double radius, double duration);

#endif
