#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>

#include "util/list.h"
#include "render/engine.h"
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

defNode(EnemyNode, Enemy);
defList(EnemyList, EnemyNode);

void activateEnemies(void);

void launchEnemySpawner(void);
void destroyEnemySpawner(void);

void clearEnemies(void);

#endif
