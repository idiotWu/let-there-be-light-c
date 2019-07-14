#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>

#include "util/list.h"
#include "engine/engine.h"
#include "maze/direction.h"

typedef struct Enemy {
  double x;
  double y;
  Direction direction;

  bool activated;

  int spriteState;
  int remainSteps;

  Animation* movingAnimation;
} Enemy;

void activateEnemies(void);

void initEnemy(void);
void destroyEnemy(void);

#endif
