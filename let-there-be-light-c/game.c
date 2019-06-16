#include "game.h"
#include "maze.h"
#include "tile.h"

void processMaze(Tile tiles[MAZE_SIZE][MAZE_SIZE], size_t pathLength) {

  for (size_t i = 0; i < MAZE_SIZE; i++) {
    for (size_t j = 0; j < MAZE_SIZE; j++) {
      if (i != MAZE_SIZE - 1 && tiles[i + 1][j] & TILE_OPEN) {
        tiles[i][j] = TILE_SHADOW;
      }
    }
  }
}
