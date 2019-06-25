#ifndef MAZE_H
#define MAZE_H

#include <stddef.h>

#include "config.h"

#include "tile.h"

int initMaze(int spawnerCount,
             int minDistance,
             int maxDistance,
             Tile tiles[MAZE_SIZE][MAZE_SIZE]);

#endif
