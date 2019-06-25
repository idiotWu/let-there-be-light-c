#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include "config.h"

#include "player.h"
#include "enemy.h"

#include "util/util.h"
#include "util/list.h"
#include "maze/tile.h"
#include "maze/direction.h"

typedef enum Scene {
  SCENE_TITLE,
  SCENE_TRANSITION,
  SCENE_GAME_STAGE,
} Scene;

typedef struct ClientRect {
  double top;
  double right;
  double bottom;
  double left;
  double width;
  double height;
} ClientRect;

typedef struct State {
  Scene currentScene;
  
  int level;
  int remainItem;

  Tile maze[MAZE_SIZE][MAZE_SIZE];
  int stepsFromPlayer[MAZE_SIZE][MAZE_SIZE];
  vec2i* openTiles;
  int pathLength;

  bool paused;
  Direction keyPressed;

  double visibleRadius;
  Player player;
  EnemyList* enemies;

  ClientRect ortho;
  ClientRect viewport;
} State;

extern State GameState;

void initGameState(void);

#endif
