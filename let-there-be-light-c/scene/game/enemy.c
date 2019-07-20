/**
 * @file
 * @brief 敵の定義
 */
#include <stdlib.h>
#include <stdbool.h>

#include "scene/game/enemy.h"

#include "state.h"

#include "render/engine.h"
#include "render/texture.h"
#include "render/fx.h"
#include "maze/tile.h"
#include "maze/maze.h"
#include "maze/floodfill.h"
#include "util/util.h"
#include "util/list.h"
#include "scene/game/difficulty.h"

//! 敵の生成器
static Animation* enemySpawner = NULL;
//! 敵のスプライトの状態（フレーム）を更新するタイマー
static Animation* enemyStateUpdater;

// ============ Explode & Spoil ============ //

/**
 * @brief 地面を凍らせる
 *
 * @param x タイルの x 座標
 * @param y タイルの y 座標
 */
static void setSpolied(int x, int y) {
  setBits(GameState.maze[y][x], TILE_SPOILED);
  fxExplode(FX_ICE_SPLIT_ROW, x + 0.5, y + 0.5, 0.5);
}

/**
 * @brief 地面を凍らせるアニメーションを更新する
 *
 * @param animation アニメーション
 */
static void spoilTilesUpdate(Animation* animation) {
  FloodState* state = animation->from;

  floodForward(state);

  ListIterator it = createListIterator(state->frontiers);

  while (!it.done) {
    Node* node = it.next(&it);
    vec2i* frontier = node->data;

    setSpolied(frontier->x, frontier->y);
  }
}

/**
 * @brief 地面を凍らせるアニメーションが終わったときの処理
 *
 * @param animation アニメーション
 */
static void spoilTilesComplete(Animation* animation) {
  FloodState* state = animation->from;
  floodDestory(state);
}

/**
 * @brief \ref floodFill アルゴリズムを使って地面を凍らせるアニメーション
 *
 * @param x      出発タイルの x 座標
 * @param y      出発タイルの y 座標
 * @param radius 凍らせる範囲
 */
static void spoilTiles(int x, int y, int radius) {
  Animation* animation = createAnimation(radius, 300 * radius / ENEMY_EXPLODE_RADIUS, 1);

  FloodState* state = floodGenerate(GameState.maze, x, y);

  animation->from = state;
  animation->cleanFlag = ANIMATION_CLEAN_NONE;

  animation->update = spoilTilesUpdate;
  animation->complete = spoilTilesComplete;

  setSpolied(x, y);
}

/**
 * @brief 敵を爆発させる
 *
 * @param enemy 敵
 */
static void enemyExplode(Enemy* enemy) {
  int radius = max(1, (double)enemy->remainSteps / ENEMY_MAX_STEPS * ENEMY_EXPLODE_RADIUS);

  cancelAnimation(enemy->movingAnimation);

  spoilTiles(enemy->x, enemy->y, radius);

  listFindDelete(GameState.enemies, enemy);
}

// ============ Spawn Enemy ============ //

/**
 * @brief 敵を移動させる
 *
 * @param enemy 敵
 */
static void moveEnemy(Enemy* enemy);

/**
 * @brief 敵がプレイヤーに近づくための変位
 *
 * @details この関数は \ref State.stepsFromPlayer "GameState.stepsFromPlayer" に基づき，
 * 敵を上下左右からプレイヤーへの距離が一番短いタイルに移動させる
 *
 * @param enemy 敵
 *
 * @return vec2i 変位
 */
static vec2i getEnemyDelta(Enemy* enemy) {
  int x = enemy->x;
  int y = enemy->y;
  int minSteps = GameState.stepsFromPlayer[y][x] - 1;

  vec2i delta = { 0, 0 };

  if (minSteps < 0) {
    // in case player is moving
    // -> move towards player
    delta.x = sign(GameState.player.x - x);
    delta.y = sign(GameState.player.y - y);

    if (!isPath(GameState.maze, x + delta.x, y)) {
      delta.x = 0;
    }

    if (!isPath(GameState.maze, x, y + delta.y)) {
      delta.y = 0;
    }

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

/**
 * @brief 敵を移動させるアニメーションを更新する
 *
 * @details 敵がプレイヤーに突き当たると爆発する
 *
 * @param animation アニメーション
 */
static void moveEnemyUpdate(Animation* animation) {
  Enemy* enemy = animation->target;
  vec2i* fromPos = animation->from;
  vec2i* delta = animation->delta;

  double percent = (double)animation->currentFrame / animation->frameCount;

  enemy->x = fromPos->x + delta->x * percent;
  enemy->y = fromPos->y + delta->y * percent;

  if (distance(enemy->x, enemy->y,
               GameState.player.x, GameState.player.y) <= 0.5) {
    enemyExplode(enemy);
    cancelAnimation(animation);
  }
}

/**
 * @brief 敵を移動させるアニメーションが終わった後の処理
 *
 * @param animation アニメーション
 */
static void moveEnemyComplete(Animation* animation) {
  Enemy* enemy = animation->target;
  enemy->remainSteps--;

  if (enemy->remainSteps == 0) {
    enemyExplode(enemy);
  } else {
    moveEnemy(enemy);
  }
}

static void moveEnemy(Enemy* enemy) {
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

  animation->target = enemy;
  animation->from = fromPos;
  animation->delta = d;

  animation->update = moveEnemyUpdate;
  animation->complete = moveEnemyComplete;
}

void activateEnemies(void) {
  Player* player = &GameState.player;
  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    Node* node = it.next(&it);
    Enemy* enemy = node->data;

    if (enemy->activated) {
      continue;
    }

    double r = distance(player->x, player->y, enemy->x, enemy->y);

    if (r <= 1.0 || r <= GameState.visibleRadius) {
      enemy->activated = true;

      fxExplodeAtTile(FX_ENEMY_REVIVE_ROW, enemy->x, enemy->y);

//      moveEnemy(node);
      enemy->movingAnimation = delay(200, (DelayCallback)moveEnemy, enemy);
    }
  }
}

// ============ Spawn Enemy ============ //

/**
 * @brief 敵を生成する
 *
 * @param animation 生成器のアニメーション（使われていない）
 */
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

  Node* node = createNode();
  node->data = enemy;

  listAppend(GameState.enemies, node);

  fxExplodeAtTile(FX_ENEMY_SPAWN_ROW, pos.x, pos.y);
}

/**
 * @brief 敵のスプライトを更新する
 *
 * @param animation 更新のタイマー（使われていない）
 */
static void updateEnemyState(Animation* animation) {
  UNUSED(animation);

  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    Node* node = it.next(&it);
    Enemy* enemy = node->data;

    if (enemy->activated) {
      enemy->spriteState = (enemy->spriteState + 1) % ENEMY_SPRITES->cols;
    } else {
      enemy->spriteState = ENEMY_INACTIVE_COL;
    }
  }
}

// ============ Misc ============ //

void initEnemy(void) {
  enemySpawner = createAnimation(1, getEnemySpawnIntervel(), ANIMATION_INFINITY);

  enemySpawner->update = createEnemy;

  enemyStateUpdater = createAnimation(ENEMY_SPRITES->cols,
                                      CHARACTER_STATE_ANIMATION_DURATION * 2,   // slow down
                                      ANIMATION_INFINITY);

  enemyStateUpdater->update = updateEnemyState;
}

void destroyEnemy(bool shouldExpload) {
  cancelAnimation(enemySpawner);
  cancelAnimation(enemyStateUpdater);

  enemySpawner = enemyStateUpdater = NULL;

  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    Node* node = it.next(&it);
    Enemy* enemy = node->data;

    cancelAnimation(enemy->movingAnimation);

    if (shouldExpload) {
      fxExplodeAtTile(FX_SMOKE_ROW, enemy->x, enemy->y);
    }

    listDelete(GameState.enemies, node);
  }
}
