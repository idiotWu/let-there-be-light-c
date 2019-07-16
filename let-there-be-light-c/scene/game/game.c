#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "glut.h"

#include "config.h"
#include "state.h"

#include "game.h"
#include "enemy.h"
#include "player.h"
#include "render.h"
#include "keyboard.h"
#include "difficulty.h"

#include "scene/game-over/game-over.h"
#include "scene/scene.h"
#include "render/transition.h"
#include "util/util.h"
#include "maze/maze.h"
#include "maze/tile.h"
#include "maze/floodfill.h"
#include "maze/direction.h"
#include "render/engine.h"
#include "render/texture.h"
#include "render/fx.h"

static void initGame(void);
static void updateGame(void);
static void leaveGame(void);

static Scene game = {
  .init = initGame,
  .update = updateGame,
  .render = renderGame,
  .destroy = leaveGame,
};

Scene* gameScene = &game;

// ============ Maze Initialization Begin ============ //

static void initItems(void) {
  Tile (*tiles)[MAZE_SIZE] = GameState.maze;

  double itemDesity = getItemDensity();

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

// ============ Visible Radius ============ //

static void expandVisionUpdate(Animation* animation) {
  double* fromRadius = animation->from;
  double* deltaRadius = animation->delta;

  double percent = (double)animation->currentFrame / animation->frameCount;

  GameState.visibleRadius = *fromRadius + *deltaRadius * percent;
}

static void expandVision(double radius, double duration) {
  double* fromRadius = malloc(sizeof(double));
  double* deltaRadius = malloc(sizeof(double));

  *fromRadius = GameState.visibleRadius;
  *deltaRadius = radius - GameState.visibleRadius;

  Animation* animation = createAnimation60FPS(duration, 1);

  animation->from = fromRadius;
  animation->delta = deltaRadius;
  animation->render = expandVisionUpdate;
}

// ============ Keyboard Handler ============ //

// handle pressed arrow keys
static void readKeyboard(void) {
  Direction moveDir = getPressedKeys();

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

// ============ Misc ============ //

static void initGame(void) {
  int spawnerCount = getSpawnerCount();
  int minDistance = getMinBuilderDistance();
  int maxDistance = minDistance * 2;

  int pathLength;
  int tried = 0;

  do {
    pathLength = initMaze(spawnerCount, minDistance, maxDistance, GameState.maze);
    tried++;
  } while(pathLength < MIN_PATH_LENGTH && tried < 5);

  free(GameState.openTiles);

  GameState.paused = false;
  GameState.pathLength = pathLength;
  GameState.openTiles = malloc(pathLength * sizeof(*GameState.openTiles));
  GameState.visibleRadius = getInitialVisibleRadius();

  initItems();
  initPlayer();
  initEnemy();

  bindGameKeyboardHandlers();
}

static void gameClear(void* _) {
  UNUSED(_);

  if (GameState.level.minor < LEVEL_INTERVAL) {
    GameState.level.minor++;
  } else {
    GameState.level.major++;
    GameState.level.minor = 1;
  }

  GameState.paused = true;

  levelTransition();
}

static void updateGame(void) {
  if (GameState.paused) {
    return;
  }

  Player* player = &GameState.player;

  player->spoiled = (GameState.maze[(int)player->y][(int)player->x] & TILE_SPOILED);

  GameState.visibleRadius -= getVisibleRadiusRuducingRate();

  if (GameState.visibleRadius <= 0) {
    return switchScene(gameOverScene);
  }

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
    destroyEnemy(true);
    fxFlood(player->x, player->y);

    delay(5000, gameClear, NULL);
  }
}

static void leaveGame(void) {
  destroyPlayer();
  destroyEnemy(false);
  removeGameKeyboardHandlers();
}
