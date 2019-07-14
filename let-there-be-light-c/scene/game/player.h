#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

#include "maze/direction.h"

typedef struct Player {
  double x;
  double y;
  bool idle;
  bool spoiled;
  int spriteState;
  Direction direction;
} Player;

void movePlayer(int deltaX, int deltaY);
void updateStepsFromPlayer(void);

void initPlayer(void);
void destroyPlayer(void);

#endif
