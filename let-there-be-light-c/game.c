#include <stdbool.h>
#include <stdlib.h>
#include "glut.h"

#include "game.h"
#include "maze.h"
#include "tile.h"
#include "util.h"
#include "state.h"
#include "config.h"
#include "engine.h"
#include "texture.h"
#include "direction.h"
#include "expr-fx.h"

static int processMaze(Tile tiles[MAZE_SIZE][MAZE_SIZE], int pathLength) {
  // item desity: (0.5, 1]
  double itemDesity = 1.0 - (GameState.level % LEVEL_INTERVAL) / LEVEL_INTERVAL * 0.5;

  int startIndex = randomInt(1, pathLength);
  int openTiles = 0;
  int itemCount = 0;
  bool startPosSet = false;

  for (size_t y = 0; y < MAZE_SIZE; y++) {
    for (size_t x = 0; x < MAZE_SIZE; x++) {
      if (tiles[y][x] & TILE_OPEN) {
        if (!startPosSet && ++openTiles == startIndex) {
          GameState.player.x = (double)x;
          GameState.player.y = (double)y;
          startPosSet = true;
        }

        // clear items randomly
        if (randomBetween(0, 1) >= itemDesity) {
          tiles[y][x] = TILE_PATH;
        } else {
          itemCount++;
        }
      } else if (y != MAZE_SIZE - 1 && tiles[y + 1][x] & TILE_OPEN) {
        tiles[y][x] = TILE_SHADOW;
      }
    }
  }

  return itemCount;
}

// ============ Player Begin ============ //

// update callback
static void playerMoveUpdate(Animation* animation) {
  int* fromPos = animation->from;
  int* delta = animation->to;

  double percent = (double)animation->currentFrame / animation->frameCount;

  GameState.player.x = fromPos[0] + delta[0] * percent;
  GameState.player.y = fromPos[1] + delta[1] * percent;
}

// complete callback
static void playerMoveComplete(Animation* animation) {
  UNUSED(animation);
  GameState.idle = true;
}

// move player by delta
static void playerMove(int deltaX, int deltaY) {
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

  int* fromPos = malloc(2 * sizeof(int));
  int* delta = malloc(2 * sizeof(int));

  fromPos[0] = GameState.player.x;
  fromPos[1] = GameState.player.y;

  delta[0] = deltaX;
  delta[1] = deltaY;

  Animation* moveAnimation = createAnimation60FPS(PLAYER_MOVE_ANIMATION_DURATION, 1);

  moveAnimation->from = fromPos;
  moveAnimation->to = delta;
  moveAnimation->update = playerMoveUpdate;
  moveAnimation->complete = playerMoveComplete;
}

// forward player's state
static void playerNextState(void) {
  GameState.player.state = (GameState.player.state + 1) % PLAYER_SPRITES->cols;
}

// check if player can move with the given delta
static bool playerCanMove(int deltaX, int deltaY) {
  int nextX = (int)GameState.player.x + deltaX;
  int nextY = (int)GameState.player.y + deltaY;

  if (nextX < 0 || nextX >= MAZE_SIZE ||
      nextY < 0 || nextY >= MAZE_SIZE) {
    return false;
  }

  return GameState.maze[nextY][nextX] & TILE_OPEN;
}

// ============ Player End ============ //

static void initPlayerDirection(void) {
  Direction* direction = &GameState.player.direction;

  if (playerCanMove(0, -1)) {
    *direction = DIR_DOWN;
  } else if (playerCanMove(0, +1)) {
    *direction = DIR_UP;
  } else if (playerCanMove(-1, 0)) {
    *direction = DIR_LEFT;
  } else if (playerCanMove(+1, 0)) {
    *direction = DIR_RIGHT;
  }
}

static void updatePlayerState(Animation* animation) {
  // slows down when game is idle (i.e. play is not moving)
  if (GameState.idle && animation->currentFrame != 1) {
    return;
  }

  playerNextState();
}

// handle pressed arrow keys
static void readKeyboard(void) {
  Direction moveDir = GameState.keyPressed;

  if (moveDir == DIR_NONE) {
    return;
  }

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

  if (!playerCanMove(deltaX, 0)) {
    deltaX = 0;
  }
  if (!playerCanMove(0, deltaY)) {
    deltaY = 0;
  }

  if (deltaX || deltaY) {
    GameState.idle = false;
    playerMove(deltaX, deltaY);
  }
}

static void expandVisionUpdate(Animation* animation) {
  double* fromRadius = animation->from;
  double* deltaRadius = animation->to;

  double percent = (double)animation->currentFrame / animation->frameCount;

  GameState.visibleRadius = *fromRadius + *deltaRadius * percent;
}

static void updateItems(void) {
  int x = GameState.player.x;
  int y = GameState.player.y;

  Tile tile = GameState.maze[y][x];

  if (tile & TILE_ITEM) {
    // pick up item
    GameState.maze[y][x] = TILE_PATH;
    GameState.remainItem--;
  } else {
    // do nothing if no item exists
    return;
  }

  if (tile == TILE_KERNEL) {
    fxGen(x, y);
    expandVision(min(GameState.visibleRadius + 1.0, MAX_VISIBLE_RADIUS), 500);
  }
}

void expandVision(double radius, double duration) {
  double* fromRadius = malloc(sizeof(double));
  double* deltaRadius = malloc(sizeof(double));

  *fromRadius = GameState.visibleRadius;
  *deltaRadius = radius - GameState.visibleRadius;

  Animation* animation = createAnimation60FPS(duration, 1);

  animation->from = fromRadius;
  animation->to = deltaRadius;
  animation->render = expandVisionUpdate;
}

void initGame(void) {
  Animation* animation = createAnimation(PLAYER_SPRITES->cols, PLAYER_STATE_ANIMATION_DURATION, ANIMATION_INFINITY);

  animation->update = updatePlayerState;
}

void buildWorld(void) {
  int spawnerCount = MIN_SPAWNER_COUNT + GameState.level / LEVEL_INTERVAL;
  int minDistance = 2 * (LEVEL_INTERVAL - GameState.level % LEVEL_INTERVAL); // 2, 4, 6, ...
  int maxDistance = minDistance * 2;

  int pathLength = initMaze(spawnerCount, minDistance, maxDistance, GameState.maze);

  GameState.remainItem = processMaze(GameState.maze, pathLength);

  initPlayerDirection();
}

void updateGame(void) {
  if (GameState.cleared) {
    return;
  }

  GameState.visibleRadius -= RADIUS_DECREASING_RATE;

  if (GameState.idle) {
    updateItems();
    readKeyboard();
  }

  if (GameState.remainItem == 0) {
    GameState.cleared = true;
    expandVision(MAZE_SIZE * 10, 3000);
  }
}
