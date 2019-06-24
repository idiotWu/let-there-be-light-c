#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include "config.h"
#include "tile.h"
#include "util.h"
#include "list.h"
#include "direction.h"

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

typedef struct Player {
  double x;
  double y;
  bool idle;
  bool spoiled;
  int spriteState;
  Direction direction;
} Player;

typedef struct Enemy {
  double x;
  double y;
  bool idle;
  int spriteState;
  bool activated;
  int remainSteps;
  Direction direction;
} Enemy;

defNode(EnemyNode, Enemy);
defList(EnemyList, EnemyNode);

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
