#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "glut.h"

#include "game.h"
#include "maze.h"
#include "tile.h"
#include "util.h"
#include "state.h"
#include "config.h"
#include "engine.h"
#include "texture.h"
#include "floodfill.h"
#include "direction.h"
#include "expr-fx.h"

// ============ Maze Initialization Begin ============ //

// check if the next position is path
static bool isPath(int nextX, int nextY) {
  if (nextX < 0 || nextX >= MAZE_SIZE ||
      nextY < 0 || nextY >= MAZE_SIZE) {
    return false;
  }

  return GameState.maze[nextY][nextX] & TILE_OPEN;
}

static void initItems(void) {
  Tile (*tiles)[MAZE_SIZE] = GameState.maze;

  // item desity: (0.5, 1]
  double itemDesity = 1.0 - (GameState.level % LEVEL_INTERVAL) / LEVEL_INTERVAL * 0.5;

  int openCount = 0;

  for (int y = 0; y < MAZE_SIZE; y++) {
    for (int x = 0; x < MAZE_SIZE; x++) {
      if (tiles[y][x] & TILE_OPEN) {
        vec2i pos = { x, y };
        GameState.openTiles[openCount++] = pos;

        // clear items randomly
        if (randomBetween(0, 1) >= itemDesity) {
          tiles[y][x] = TILE_PATH;
        } else {
          GameState.remainItem++;
        }
      } else if (y != MAZE_SIZE - 1 && tiles[y + 1][x] & TILE_OPEN) {
        tiles[y][x] = TILE_SHADOW;
      }
    }
  }
}

static void initStartPos(void) {
  int startIndex = randomInt(0, GameState.pathLength - 1);

  vec2i startPos = GameState.openTiles[startIndex];

  GameState.player.x = startPos.x;
  GameState.player.y = startPos.y;
}

static Direction getAvailableDirection(int x, int y) {
  Direction direction = DIR_NONE;

  if (isPath(x, y - 1)) {
    direction = DIR_DOWN;
  } else if (isPath(x, y + 1)) {
    direction = DIR_UP;
  } else if (isPath(x - 1, y)) {
    direction = DIR_LEFT;
  } else if (isPath(x + 1, y)) {
    direction = DIR_RIGHT;
  }

  return direction;
}

static void processMaze(void) {
  initItems();
  initStartPos();

  GameState.player.direction = getAvailableDirection(GameState.player.x, GameState.player.y);
}

// ============ Maze Initialization End ============ //

// ============ Player Begin ============ //

// update callback
static void movePlayerUpdate(Animation* animation) {
  vec2i* fromPos = animation->from;
  vec2i* delta = animation->delta;

  double percent = (double)animation->currentFrame / animation->frameCount;

  GameState.player.x = fromPos->x + delta->x * percent;
  GameState.player.y = fromPos->y + delta->y * percent;
}

// complete callback
static void movePlayerComplete(Animation* animation) {
  UNUSED(animation);
  GameState.player.idle = true;
}

// move player by delta
static void movePlayer(int deltaX, int deltaY) {
  // ensure single direction
  if (deltaX && deltaY) {
    deltaY = 0;
  }

  if (deltaX > 0) {
    GameState.player.direction = DIR_RIGHT;
  } else if (deltaX < 0) {
    GameState.player.direction = DIR_LEFT;
  }

  if (deltaY > 0) {
    GameState.player.direction = DIR_UP;
  } else if (deltaY < 0) {
    GameState.player.direction = DIR_DOWN;
  }

  vec2i* fromPos = malloc(sizeof(vec2i));
  vec2i* delta = malloc(sizeof(vec2i));

  fromPos->x = GameState.player.x;
  fromPos->y = GameState.player.y;

  delta->x = deltaX;
  delta->y = deltaY;

  Animation* moveAnimation = createAnimation60FPS(CHARACTER_MOVE_ANIMATION_DURATION, 1);

  moveAnimation->from = fromPos;
  moveAnimation->delta = delta;
  moveAnimation->update = movePlayerUpdate;
  moveAnimation->complete = movePlayerComplete;
}

// ============ Player End ============ //

// ============ Enemy Begin ============ //
static void spawnEnemy(Animation* animation) {
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

  if (GameState.maze[pos.y][pos.x] == TILE_KERNEL) {
    return;
  }

  Enemy* enemy = calloc(1, sizeof(Enemy));
  enemy->x = pos.x;
  enemy->y = pos.y;
  enemy->spriteState = ENEMY_INACTIVE_COL;
  enemy->idle = true;
  enemy->activated = false;
  enemy->direction = getAvailableDirection(pos.x, pos.y);
  enemy->remainSteps = randomInt(ENEMY_MIN_STEPS, ENEMY_MAX_STEPS);

  EnemyNode* node = createNode();
  node->data = enemy;

  listAppend(GameState.enemies, node);

  fxExplodeGen(FX_ENEMY_SPAWN_ROW, pos.x, pos.y);
}

// flood-fill path finding
static vec2i getEnemyDelta(Enemy* enemy) {
  int depthMap[MAZE_SIZE][MAZE_SIZE] = { 0 };
  FloodState* state = floodGenerate(GameState.maze, enemy->x, enemy->y);

  bool reached = false;

  int dx = GameState.player.x;
  int dy = GameState.player.y;

  while (!reached && !state->finished) {
    floodForward(state);

    ListIterator it = createListIterator(state->frontiers);

    while (!it.done) {
      FrontierNode* node = it.next(&it);
      Frontier* f = node->data;

      depthMap[f->y][f->x] = state->depth;

      if (f->x == dx && f->y == dy) {
        reached = true;
        break;
      }
    }
  }

//  for (int i = MAZE_SIZE - 1; i >= 0; i--) {
//    for (int j = 0; j < MAZE_SIZE; j++) {
//      printf("%2d ", weight[i][j]);
//    }
//    printf("\n");
//  }

  int depth = state->depth;

  while (depth != 1) {
    depth--;

    if (depthMap[dy][dx - 1] == depth) {
      dx -= 1;
    } else if (depthMap[dy][dx + 1] == depth) {
      dx += 1;
    } else if (depthMap[dy - 1][dx] == depth) {
      dy -= 1;
    } else if (depthMap[dy + 1][dx] == depth) {
      dy += 1;
    }

//    printf("%2d: (%2d, %2d)\n", currentWeight, dx, dy);
  }

  floodDestory(state);

  vec2i delta = {
    dx - enemy->x,
    dy - enemy->y
  };

  assert(abs(delta.x) + abs(delta.y) == 1);

  return delta;
}

static void clearEnemy(void) {
  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    EnemyNode* node = it.next(&it);
    Enemy* enemy = node->data;

    fxExplodeGen(FX_SMOKE_ROW, enemy->x, enemy->y);

    listDelete(GameState.enemies, node);
  }
}

static void spoilTilesUpdate(Animation* animation) {
  FloodState* state = animation->from;

  floodForward(state);

  ListIterator it = createListIterator(state->frontiers);

  while (!it.done) {
    FrontierNode* node = it.next(&it);
    Frontier* f = node->data;

    setBit(&GameState.maze[f->y][f->x], TILE_SPOILED);
  }
}

static void spoilTilesComplete(Animation* animation) {
  FloodState* state = animation->from;
  floodDestory(state);
}

static void spoilTiles(int x, int y) {
  setBit(&GameState.maze[y][x], TILE_SPOILED);

  Animation* animation = createAnimation(ENEMY_EXPLODE_RADIUS, 300, 1);

  FloodState* state = floodGenerate(GameState.maze, x, y);

  animation->from = state;
  animation->cleanFlag = ANIMATION_CLEAN_NONE;

  animation->update = spoilTilesUpdate;
  animation->complete = spoilTilesComplete;
}

static void enemyExplode(Enemy* enemy) {
  fxExplodeGen(FX_ICE_SPLIT_ROW, enemy->x, enemy->y);
  spoilTiles(enemy->x, enemy->y);
}

// move enemy update callback
static void moveEnemyUpdate(Animation* animation) {
  // in case enemy are cleared
  if (!GameState.enemies->count) {
    cancelAnimation(animation);
    return;
  }

  Enemy* enemy = animation->target;
  vec2i* fromPos = animation->from;
  vec2i* delta = animation->delta;

  double percent = (double)animation->currentFrame / animation->frameCount;

  enemy->x = fromPos->x + delta->x * percent;
  enemy->y = fromPos->y + delta->y * percent;
}

// move enemy complete callback
static void moveEnemyComplete(Animation* animation) {
  Enemy* enemy = animation->target;
  enemy->remainSteps--;
  enemy->idle = true;
}

static void moveEnemy(Enemy* enemy) {
  if (!enemy->activated || !enemy->idle) {
    return;
  }

  enemy->idle = false;

  vec2i delta = getEnemyDelta(enemy);
  enemy->direction = deltaToDirection(delta);

  Animation* animation = createAnimation60FPS(CHARACTER_MOVE_ANIMATION_DURATION * ENEMY_SLOW_DOWN, 1);

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

static bool shouldExplodeEnemy(Enemy* enemy) {
  if (enemy->remainSteps == 0) {
    return true;
  }

  return (int)enemy->x == (int)GameState.player.x && (int)enemy->y == (int)GameState.player.y;
}

static void updateAllEnemies(void) {
  Player* player = &GameState.player;
  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    EnemyNode* node = it.next(&it);
    Enemy* enemy = node->data;

    if (enemy->idle && shouldExplodeEnemy(enemy)) {
      enemyExplode(enemy);
      listDelete(GameState.enemies, node);
      continue;
    }

    if (!enemy->activated) {
      double r = distance(player->x, player->y, enemy->x, enemy->y);

      if (r <= GameState.visibleRadius) {
        enemy->activated = true;
        fxExplodeGen(FX_ENEMY_REVIVE_ROW, enemy->x, enemy->y);
      }
    }

    if (enemy->activated && enemy->idle) {
      moveEnemy(enemy);
    }
  }
}

// ============ Enemy End ============ //

// ============ State Update Begin ============ //

// forward player's state
static void updatePlayerState(Animation* animation) {
  // slows down when player is idle (i.e. play is not moving)
  if (GameState.player.idle && animation->currentFrame != 1) {
    return;
  }

  GameState.player.spriteState = (GameState.player.spriteState + 1) % PLAYER_SPRITES->cols;
}

static void updateEnemyState(Animation* animation) {
//  UNUSED(animation);
  if (animation->currentFrame % 2 != 0) {
    return;
  }

  ListIterator it = createListIterator(GameState.enemies);

  while (!it.done) {
    EnemyNode* node = it.next(&it);
    Enemy* enemy = node->data;

    if (enemy->activated) {
      enemy->spriteState = (enemy->spriteState + 1) % ENEMY_SPRITES->cols;
    } else {
      enemy->spriteState = ENEMY_INACTIVE_COL;
    }
  }
}

static void updateCharacterState(Animation* animation) {
  updatePlayerState(animation);
  updateEnemyState(animation);
}

// ============ State Update End ============ //

// handle pressed arrow keys
static void readKeyboard(void) {
  Direction moveDir = GameState.keyPressed;

  if (moveDir == DIR_NONE) {
    return;
  }

  int x = GameState.player.x;
  int y = GameState.player.y;

  int deltaX = 0;
  int deltaY = 0;

  if (moveDir & DIR_LEFT) {
    deltaX--;
  }
  if (moveDir & DIR_RIGHT) {
    deltaX++;
  }
  if (moveDir & DIR_UP) {
    deltaY++;
  }
  if (moveDir & DIR_DOWN) {
    deltaY--;
  }

  if (!isPath(x + deltaX, y)) {
    deltaX = 0;
  }
  if (!isPath(x, y + deltaY)) {
    deltaY = 0;
  }

  if (deltaX || deltaY) {
    GameState.player.idle = false;
    movePlayer(deltaX, deltaY);
  }
}

static void expandVisionUpdate(Animation* animation) {
  double* fromRadius = animation->from;
  double* deltaRadius = animation->delta;

  double percent = (double)animation->currentFrame / animation->frameCount;

  GameState.visibleRadius = *fromRadius + *deltaRadius * percent;
}

static void updateItems(void) {
  int x = GameState.player.x;
  int y = GameState.player.y;

  Tile* tile = &GameState.maze[y][x];

  if (*tile & TILE_ITEM) {
    // pick up item
    if (*tile & TILE_KERNEL) {
      fxFloodGen(x, y);
      expandVision(min(GameState.visibleRadius + 1.0, MAX_VISIBLE_RADIUS), 500);
    }

    clearBit(tile, TILE_ITEM);
    setBit(tile, TILE_PATH);
    GameState.remainItem--;
  }
}

void expandVision(double radius, double duration) {
  double* fromRadius = malloc(sizeof(double));
  double* deltaRadius = malloc(sizeof(double));

  *fromRadius = GameState.visibleRadius;
  *deltaRadius = radius - GameState.visibleRadius;

  Animation* animation = createAnimation60FPS(duration, 1);

  animation->from = fromRadius;
  animation->delta = deltaRadius;
  animation->render = expandVisionUpdate;
}

void initGame(void) {
  // update character state
  Animation* characterStateUpdater = createAnimation(PLAYER_SPRITES->cols, CHARACTER_STATE_ANIMATION_DURATION, ANIMATION_INFINITY);

  characterStateUpdater->update = updateCharacterState;

  // spawn enemy
  Animation* enemySpawner = createAnimation(1, ENEMY_SPAWN_INTERVEL, ANIMATION_INFINITY);
  enemySpawner->update = spawnEnemy;
}

void buildWorld(void) {
  int spawnerCount = MIN_SPAWNER_COUNT + GameState.level / LEVEL_INTERVAL;
  int minDistance = 2 * (LEVEL_INTERVAL - GameState.level % LEVEL_INTERVAL); // 2, 4, 6, ...
  int maxDistance = minDistance * 2;

  int pathLength;
  int tried = 0;

  do {
    pathLength = initMaze(spawnerCount, minDistance, maxDistance, GameState.maze);
    tried++;
  } while(pathLength < MIN_PATH_LENGTH && tried < 5);

  free(GameState.openTiles);
  GameState.pathLength = pathLength;
  GameState.openTiles = malloc(pathLength * sizeof(*GameState.openTiles));

  processMaze();
}

void updateGame(void) {
  if (GameState.paused) {
    return;
  }

  Player* player = &GameState.player;

  player->spoiled = (GameState.maze[(int)player->y][(int)player->x] & TILE_SPOILED);

  GameState.visibleRadius -= RADIUS_DECREASING_RATE * (player->spoiled ? SPOILED_DAMAGE : 1.0);

  if (GameState.player.idle) {
    updateItems();
    readKeyboard();
  }

  updateAllEnemies();

  if (GameState.remainItem == 0) {
    GameState.paused = true;
    player->spoiled = false;
    expandVision(MAZE_SIZE * 10, 3000);
    clearEnemy();
    fxFloodGen(player->x, player->y);
  }
}
