#include <stdbool.h>

#include "game.h"
#include "maze.h"
#include "tile.h"
#include "util.h"
#include "state.h"
#include "config.h"
#include "direction.h"

static void processMaze(Tile tiles[MAZE_SIZE][MAZE_SIZE], int pathLength) {
  int idx = randomInt(1, pathLength);
  int openTiles = 0;
  bool startPosSet = false;

  for (size_t y = 0; y < MAZE_SIZE; y++) {
    for (size_t x = 0; x < MAZE_SIZE; x++) {
      if (tiles[y][x] & TILE_OPEN) {
        if (!startPosSet && ++openTiles == idx) {
          GameState.player.x = (double)x;
          GameState.player.y = (double)y;
          startPosSet = true;
        }
      } else if (y != MAZE_SIZE - 1 && tiles[y + 1][x] & TILE_OPEN) {
        tiles[y][x] = TILE_SHADOW;
      }
    }
  }
}

static bool canMove(int deltaX, int deltaY) {
  int nextX = (int)GameState.player.x + deltaX;
  int nextY = (int)GameState.player.y + deltaY;

  if (nextX < 0 || nextX >= MAZE_SIZE ||
      nextY < 0 || nextY >= MAZE_SIZE) {
    return false;
  }

  return GameState.map[nextY][nextX] & TILE_OPEN;
}

static void initPlayerDirection() {
  Direction direction = DIR_DOWN;

  if (canMove(0, -1)) {
    direction = DIR_DOWN;
  } else if (canMove(0, +1)) {
    direction = DIR_UP;
  } else if (canMove(-1, 0)) {
    direction = DIR_LEFT;
  } else if (canMove(+1, 0)) {
    direction = DIR_RIGHT;
  }

  GameState.player.direction = direction;
}

void buildWorld(int level) {
  int pathLength = initMaze(4, 4, 10, GameState.map);
  processMaze(GameState.map, pathLength);
  initPlayerDirection();

  GameState.level = level;
  GameState.visibleRadius = 5;
}
