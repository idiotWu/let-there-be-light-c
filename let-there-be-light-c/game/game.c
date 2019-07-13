#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "glut.h"

#include "config.h"

#include "game.h"
#include "state.h"
#include "player.h"
#include "enemy.h"

#include "util/util.h"
#include "maze/maze.h"
#include "maze/tile.h"
#include "maze/floodfill.h"
#include "maze/direction.h"
#include "render/fx.h"
#include "render/engine.h"
#include "render/texture.h"

// ============ Maze Initialization Begin ============ //

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
          clearBits(tiles[y][x], TILE_ITEM);
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

  GameState.player.direction = getAvailableDirection(GameState.maze, startPos.x, startPos.y);
}

// ============ State Updater Begin ============ //

// forward player's state
static void updatePlayerState(Animation* animation) {
  // slows down when player is idle (i.e. play is not moving)
  if (GameState.player.idle && animation->currentFrame != 1) {
    return;
  }

  GameState.player.spriteState = (GameState.player.spriteState + 1) % PLAYER_SPRITES->cols;
}

static void updateEnemyState(Animation* animation) {
  // TODO: slow down enemy
  if (animation->currentFrame % 2 != 0) {
    return;
  }

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

static void updateCharacterState(Animation* animation) {
  updatePlayerState(animation);
  updateEnemyState(animation);
}

// ============ Keyboard Handler ============ //

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

  if (!isPath(GameState.maze, x + deltaX, y)) {
    deltaX = 0;
  }
  if (!isPath(GameState.maze, x, y + deltaY)) {
    deltaY = 0;
  }

  if (deltaX || deltaY) {
    GameState.player.idle = false;
    movePlayer(deltaX, deltaY);
  }
}

static void updateItems(void) {
  int x = GameState.player.x;
  int y = GameState.player.y;

  Tile tile = GameState.maze[y][x];

  if (tile & TILE_ITEM) {
    // pick up item
    if (tile & TILE_KERNEL) {
      fxFlood(x, y);
      expandVision(min(GameState.visibleRadius + 1.0, MAX_VISIBLE_RADIUS), 500);
    }

    clearBits(GameState.maze[y][x], TILE_ITEM);
    GameState.remainItem--;
  }
}

// ============ Visible Radius ============ //

static void expandVisionUpdate(Animation* animation) {
  double* fromRadius = animation->from;
  double* deltaRadius = animation->delta;

  double percent = (double)animation->currentFrame / animation->frameCount;

  GameState.visibleRadius = *fromRadius + *deltaRadius * percent;
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

// ============ Set Level ============ //

static void enterNextLevel(void* _) {
  UNUSED(_);

  buildWorld();

  GameState.paused = false;
  GameState.currentScene = SCENE_GAME_STAGE;

  fxDoorOpen(300);
}

static void showLevelTile(Animation* _) {
  UNUSED(_);
  GameState.currentScene = SCENE_STAGE_TITLE;

  delay(1000, enterNextLevel, NULL);
}

void nextLevel(int level) {
  GameState.level = level;
  GameState.paused = true;

  Animation* animation = fxDoorClose(300);
  animation->complete = showLevelTile;
}

// ============ TODO REFACTOR ============ //

void initGame(void) {
  // update character state
  Animation* characterStateUpdater = createAnimation(PLAYER_SPRITES->cols, CHARACTER_STATE_ANIMATION_DURATION, ANIMATION_INFINITY);

  characterStateUpdater->update = updateCharacterState;
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
  GameState.visibleRadius = INITIAL_VISIBLE_RADIUS + GameState.lastVisibleRadius / 2.0;

  initItems();
  initStartPos();

  launchEnemySpawner();
  updateStepsFromPlayer();
}

static void gameClear(void* _) {
  UNUSED(_);
  nextLevel(GameState.level + 1);
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

  activateEnemies();

  if (GameState.remainItem == 0) {
    player->spoiled = false;
    GameState.paused = true;
    GameState.lastVisibleRadius = GameState.visibleRadius;
    
    expandVision(MAZE_SIZE * 10, 3000);
    clearEnemies();
    fxFlood(player->x, player->y);

    delay(5000, gameClear, NULL);
  }
}
