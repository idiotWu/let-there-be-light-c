#ifndef MAZE_H
#define MAZE_H

#include <stddef.h>
#include <stdbool.h>

#include "config.h"

#include "maze/tile.h"
#include "maze/direction.h"

int initMaze(int spawnerCount,
             int minDistance,
             int maxDistance,
             Tile tiles[MAZE_SIZE][MAZE_SIZE]);

bool isPath(Tile tiles[MAZE_SIZE][MAZE_SIZE],
            int x, int y);

Direction getAvailableDirection(Tile tiles[MAZE_SIZE][MAZE_SIZE],
                                int x, int y);

#endif
