#include <stdlib.h>

#include "enemy.h"
#include "state.h"

#include "render/fx.h"
#include "render/engine.h"
#include "render/texture.h"
#include "maze/tile.h"
#include "maze/maze.h"
#include "maze/floodfill.h"
#include "util/util.h"

static Animation* enemySpawner = NULL;

// ============ Explode & Spoil ============ //

static void spoilTilesUpdate(Animation* animation) {
  FloodState* state = animation->from;

  floodForward(state);

  ListIterator it = createListIterator(state->frontiers);

  while (!it.done) {
    FrontierNode* node = it.next(&it);
    Frontier* f = node->data;

    setBits(GameState.maze[f->y][f->x], TILE_SPOILED);
    fxExplodeGen(FX_ICE_SPLIT_ROW, f->x, f->y);
  }
}

static void spoilTilesComplete(Animation* animation) {
  FloodState* state = animation->from;
  floodDestory(state);
}

static void spoilTiles(int x, int y, int radius) {
  setBits(GameState.maze[y][x], TILE_SPOILED);

  Animation* animation = createAnimation(radius, 300 * radius / ENEMY_EXPLODE_RADIUS, 1);

  FloodState* state = floodGenerate(GameState.maze, x, y);

  animation->from = state;
  animation->cleanFlag = ANIMATION_CLEAN_NONE;

  animation->update = spoilTilesUpdate;
  animation->complete = spoilTilesComplete;
}

static void enemyExplode(EnemyNode* node) {
  Enemy* enemy = node->data;
  int radius = max(1, (double)enemy->remainSteps / ENEMY_MAX_STEPS * ENEMY_EXPLODE_RADIUS);

  cancelAnimation(enemy->movingAnimation);

  fxExplodeGen(FX_ICE_SPLIT_ROW, enemy->x, enemy->y);

  spoilTiles(enemy->x, enemy->y, radius);

  listDelete(GameState.enemies, node);
}

// ============ Spawn Enemy ============ //

static void moveEnemy(EnemyNode* node);

// flood-fill path finding
static vec2i getEnemyDelta(Enemy* enemy) {
  int x = enemy->x;
  int y = enemy->y;
  int minSteps = GameState.stepsFromPlayer[y][x] - 1;

  vec2i delta = { 0, 0 };

  if (minSteps < 0) {
    return delta;
  }

  if (GameState.stepsFromPlayer[y][x - 1] == minSteps) {
    delta.x = -1;
  } else if (GameState.stepsFromPlayer[y][x + 1] == minSteps) {
    delta.x = 1;
  } else if (GameState.stepsFromPlayer[y - 1][x] == minSteps) {
    delta.y = -1;
  } else if (GameState.stepsFromPlayer[y + 1][x] == minSteps) {
    delta.y = 1;
  }

  return delta;
}

// move enemy update callback
static void moveEnemyUpdate(Animation* animation) {
  EnemyNode* node = animation->target;
  Enemy* enemy = node->data;
  vec2i* fromPos = animation->from;
  vec2i* delta = animation->delta;

  double percent = (double)animation->currentFrame / animation->frameCount;

  enemy->x = fromPos->x + delta->x * percent;
  enemy->y = fromPos->y + delta->y * percent;

  if (distance(enemy->x, enemy->y,
               GameState.player.x, GameState.player.y) <= 0.5) {
    enemyExplode(node);
    cancelAnimation(animation);
  }
}

// move enemy complete callback
static void moveEnemyComplete(Animation* animation) {
  EnemyNode* node = animation->target;
  Enemy* enemy = node->data;
  enemy->remainSteps--;

  if (enemy->remainSteps == 0) {
    enemyExplode(node);
  } else {
    moveEnemy(node);
  }
}

static void moveEnemy(EnemyNode* node) {
  Enemy* enemy = node->data;

  if (!enemy->activated) {
    return;
  }

  vec2i delta = getEnemyDelta(enemy);

  enemy->direction = deltaToDirection(delta);

  Animation* animation = createAnimation60FPS(CHARACTER_MOVE_ANIMATION_DURATION * ENEMY_SLOW_DOWN, 1);

  enemy->movingAnimation = animation;

  vec2i* fromPos = malloc(sizeof(vec2i));
  vec2i* d = malloc(sizeof(vec2i));

  fromPos->x = enemy->x;
  fromPos->y = enemy->y;

  d->x = delta.x;
  d->y = delta.y;

  animation->target = node;
  animation->from = fromPos;
  animation->delta = d;

  animation->update = moveEnemyUpdate;
  animation->complete = moveEnemyComplete;
}

void activateEnemies(void) {
  Player* player = &GameState.player;
  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    EnemyNode* node = it.next(&it);
    Enemy* enemy = node->data;

    if (enemy->activated) {
      continue;
    }

    double r = distance(player->x, player->y, enemy->x, enemy->y);

    if (r <= GameState.visibleRadius) {
      enemy->activated = true;

      fxExplodeGen(FX_ENEMY_REVIVE_ROW, enemy->x, enemy->y);

      moveEnemy(node);
    }
  }
}

// ============ Spawn Enemy ============ //

static void createEnemy(Animation* animation) {
  UNUSED(animation);

  if (GameState.paused) {
    return;
  }

  int idx = randomInt(0, GameState.pathLength - 1);

  vec2i pos = GameState.openTiles[idx];

  if (pos.x == (int)GameState.player.x &&
      pos.y == (int)GameState.player.y) {
    return;
  }

  Enemy* enemy = calloc(1, sizeof(Enemy));
  enemy->x = pos.x;
  enemy->y = pos.y;
  enemy->spriteState = ENEMY_INACTIVE_COL;
  enemy->activated = false;
  enemy->movingAnimation = NULL;
  enemy->direction = getAvailableDirection(GameState.maze, pos.x, pos.y);
  enemy->remainSteps = randomInt(ENEMY_MIN_STEPS, ENEMY_MAX_STEPS);

  EnemyNode* node = createNode();
  node->data = enemy;

  listAppend(GameState.enemies, node);

  fxExplodeGen(FX_ENEMY_SPAWN_ROW, pos.x, pos.y);
}

void launchEnemySpawner(void) {
  // cleanning
  destroyEnemySpawner();

  // TODO: adjust speed
  enemySpawner = createAnimation(1, ENEMY_SPAWN_INTERVEL, ANIMATION_INFINITY);

  enemySpawner->update = createEnemy;
}

void destroyEnemySpawner(void) {
  cancelAnimation(enemySpawner);
  enemySpawner = NULL;
}

void clearEnemies(void) {
  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    EnemyNode* node = it.next(&it);
    Enemy* enemy = node->data;

    cancelAnimation(enemy->movingAnimation);

    fxExplodeGen(FX_SMOKE_ROW, enemy->x, enemy->y);

    listDelete(GameState.enemies, node);
  }
}
