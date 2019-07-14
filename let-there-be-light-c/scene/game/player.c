#include <string.h>
#include <stdlib.h>

#include "player.h"
#include "state.h"

#include "maze/floodfill.h"
#include "engine/engine.h"
#include "engine/texture.h"
#include "util/util.h"
#include "maze/direction.h"
#include "maze/maze.h"

static Animation* playerIdleStateUpdater;

// ============ Move Player Begin ============ //

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

// ============ Misc Begin ============ //

// forward player's state
static void updatePlayerState(Animation* animation) {
  // slows down when player is idle (i.e. play is not moving)
  // -> update once per period
  if (GameState.player.idle && animation->currentFrame != 1) {
    return;
  }

  GameState.player.spriteState = (GameState.player.spriteState + 1) % PLAYER_SPRITES->cols;
}

static void initStartPos(void) {
  int startIndex = randomInt(0, GameState.pathLength - 1);

  vec2i startPos = GameState.openTiles[startIndex];

  GameState.player.x = startPos.x;
  GameState.player.y = startPos.y;

  GameState.player.direction = getAvailableDirection(GameState.maze, startPos.x, startPos.y);
}

void initPlayer(void) {
  initStartPos();
  updateStepsFromPlayer();

  playerIdleStateUpdater = createAnimation(PLAYER_SPRITES->cols, CHARACTER_STATE_ANIMATION_DURATION, ANIMATION_INFINITY);

  playerIdleStateUpdater->update = updatePlayerState;
}

void destroyPlayer(void) {
  cancelAnimation(playerIdleStateUpdater);
  playerIdleStateUpdater = NULL;
}

void updateStepsFromPlayer(void) {
  FloodState* state = floodFill(GameState.maze, GameState.player.x, GameState.player.y);

  memcpy(GameState.stepsFromPlayer, state->depthMap, sizeof(state->depthMap));

  floodDestory(state);
}
