#include <string.h>
#include <stdlib.h>

#include "player.h"
#include "state.h"

#include "maze/floodfill.h"
#include "render/engine.h"
#include "util/util.h"

void updateStepsFromPlayer(void) {
  FloodState* state = floodFill(GameState.maze, GameState.player.x, GameState.player.y);

  memcpy(GameState.stepsFromPlayer, state->depthMap, sizeof(state->depthMap));

  floodDestory(state);

  //  for (int i = MAZE_SIZE - 1; i >= 0; i--) {
  //    for (int j = 0; j < MAZE_SIZE; j++) {
  //      printf("%2d ", GameState.stepsFromPlayer[i][j]);
  //    }
  //    printf("\n");
  //  }

}

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
  updateStepsFromPlayer();
}

// move player by delta
void movePlayer(int deltaX, int deltaY) {
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

